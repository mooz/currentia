// -*- c++ -*-

#include "currentia/core/thread.h"
#include "currentia/server/server.h"

#include "currentia/core/stream.h"
#include "currentia/core/relation.h"

#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/operator-simple-relation-join.h"
#include "currentia/core/operator/operator-selection.h"
#include "currentia/core/operator/operator-election.h"

#include "thirdparty/cmdline.h"

#include <iostream>

using namespace currentia;

#define LOG(x) std::cout << x << std::endl

Tuple::ptr_t create_purchase_tuple(int goods_id, int user_id);

//
// 実験で変えるべきパラメータ
//
// - 入力ストリーム達
//
//   - 数, それぞれのスキーマ
//
// - オペレータの処理内容 (入力ストリーム達に依存)
//
//   - 出力は一つでスループットを計測しようか
//
// - 属性の分布: zipf 分布
//

static Operator::ptr_t query_ptr;
static Stream::ptr_t purchase_stream;

static Relation::ptr_t goods_relation;

// ============================================================
// Begin Helpers
// ======================================================== {{{

void
create_condition_from_string(const std::string& condition_string)
{
    std::stringstream is(condition_string);
    Condition::ptr_t create_node_ptr = Parser::parse_conditions_from_stream(is);
}

Schema::ptr_t
create_schema_from_attributes(std::list<Attribute> attributes)
{
    Schema::ptr_t schema_ptr(new Schema);

    std::list<Attribute>::const_iterator attribute_iter = attributes.begin();
    for (; attribute_iter != attributes.end(); ++attribute_iter) {
        schema_ptr->add_attribute(attribute_iter->name,
                                  attribute_iter->type);
    }

    schema_ptr->freeze();

    return schema_ptr;
}

Schema::ptr_t
create_schema_from_string(const std::string& input_ddl)
{
    std::stringstream create_statement(input_ddl);
    CreateNode::ptr_t create_node_ptr = Parser::parse_create_from_stream(
        create_statement
    );
    std::list<Attribute> attributes = create_node_ptr->attributes;

    return create_schema_from_attributes(attributes);
}

Stream::ptr_t
create_stream_from_schema(Schema::ptr_t schema_ptr)
{
    // create stream
    return Stream::ptr_t(new Stream(schema_ptr));
}

Stream::ptr_t
create_stream_from_string(const std::string& input_ddl)
{
    return create_stream_from_schema(create_schema_from_string(input_ddl));
}

// }}} ========================================================
// End Helpers
// ============================================================

// ============================================================
// Begin Body
// ======================================================== {{{

// Parameters
static int PURCHASE_COUNT;
static int GOODS_COUNT;
static int AGGREGATION_WINDOW_WIDTH;

static useconds_t UPDATE_INTERVAL;

void* update_status_thread_body(void* argument)
{
    // Schema::ptr_t schema_ptr = purchase_stream->get_schema_ptr();

    // for (int i = 0; i < PURCHASE_COUNT; ++i) {
    //     purchase_stream->enqueue(create_purchase_tuple(i % GOODS_COUNT /* goods id */, rand() /* user id */));
    //     if (UPDATE_INTERVAL > 0)
    //         usleep(UPDATE_INTERVAL);
    // }

    // purchase_stream->enqueue(Tuple::create_eos()); // Finish!

    return NULL;
}

void* process_stream_thread_body(void* argument)
{
    double selected_purchase_count = 0;

    try {
        while (true) {
            Tuple::ptr_t next_tuple = query_ptr->next();
            if (next_tuple->is_eos())
                break;
            std::cout << next_tuple->toString() << std::endl;
            selected_purchase_count++;
        }
    } catch (const char* error_message) {
        std::cout << "Error while processing stream: " << error_message << std::endl;
    }

    double selectivity = static_cast<double>(selected_purchase_count) / static_cast<double>(PURCHASE_COUNT);

    std::cerr << "Selectivity :: " << selectivity << std::endl;

    return NULL;
}

// Send purchase data
static useconds_t PURCHASE_STREAM_INTERVAL;
void* stream_sending_thread_body(void* argument)
{
    Schema::ptr_t schema_ptr = purchase_stream->get_schema_ptr();

    for (int i = 0; i < PURCHASE_COUNT; ++i) {
        purchase_stream->enqueue(create_purchase_tuple(i % GOODS_COUNT /* goods id */, rand() /* user id */));
        if (PURCHASE_STREAM_INTERVAL > 0)
            usleep(PURCHASE_STREAM_INTERVAL);
    }

    purchase_stream->enqueue(Tuple::create_eos()); // Finish!

    return NULL;
}

// }}} ========================================================
// End Body
// ============================================================

// ============================================================
// Purchase
// ======================================================== {{{

static std::string PURCHASE_SCHEMA_DEFINITION;
static Schema::ptr_t purchase_schema_ptr;
void create_purchase_schema()
{
    purchase_schema_ptr = create_schema_from_string(PURCHASE_SCHEMA_DEFINITION);
}
Tuple::ptr_t create_purchase_tuple(int goods_id, int user_id)
{
    Tuple::data_t data;
    data.push_back(Object(goods_id));
    data.push_back(Object(user_id));

    return Tuple::create(purchase_schema_ptr, data);
}

// }}} ========================================================
// Purchase
// ============================================================

// ============================================================
// Goods
// ======================================================== {{{

static std::string GOODS_SCHEMA_DEFINITION;
static Schema::ptr_t goods_schema_ptr;
void create_goods_schema()
{
    goods_schema_ptr = create_schema_from_string(GOODS_SCHEMA_DEFINITION);
}
Tuple::ptr_t create_goods_tuple(int id, int price)
{
    Tuple::data_t data;
    data.push_back(Object(id));
    data.push_back(Object(price));
    return Tuple::create(goods_schema_ptr, data);
}

static int MIN_PRICE;
static int MAX_PRICE;
int generate_goods_price(int id)
{
    return MIN_PRICE + rand() % (MAX_PRICE - MIN_PRICE);
}

void set_goods_relation()
{
    create_goods_schema();

    goods_relation.reset(new Relation(goods_schema_ptr));

    for (int goods_id = 0; goods_id < GOODS_COUNT; ++goods_id) {
        goods_relation->insert(create_goods_tuple(goods_id, generate_goods_price(goods_id)));
    }
}

// }}} ========================================================
// Goods
// ============================================================

void set_purchase_stream()
{
    create_purchase_schema();
    purchase_stream = create_stream_from_schema(purchase_schema_ptr);
}

static std::string SELECTION_CONDITION;

void setup_query()
{
    Operator::ptr_t purchase_stream_current;

    Operator::ptr_t purchase_stream_adapter(new OperatorStreamAdapter(purchase_stream));
    {
        // join-relation
        OperatorSimpleRelationJoin::ptr_t relation_join(
            new OperatorSimpleRelationJoin(
                purchase_stream_adapter,
                "GOODS_ID",     // STREAM
                goods_relation,
                "ID"            // RELATION
            )
        );

        // selection
        std::stringstream is_selection(SELECTION_CONDITION);
        Condition::ptr_t selection_condition = Parser::parse_conditions_from_stream(is_selection);
        OperatorSelection::ptr_t selection(new OperatorSelection(relation_join, selection_condition));

        // election (aggregate)
        OperatorElection::ptr_t election(new OperatorElection(selection, AGGREGATION_WINDOW_WIDTH));

        purchase_stream_current = election;
    }

    query_ptr = purchase_stream_current;
}

void initialize(cmdline::parser& cmd_parser)
{
    srand(1);

    set_purchase_stream();
    set_goods_relation();
    // query
    setup_query();
}

void set_parameters_from_option(cmdline::parser& cmd_parser)
{
    UPDATE_INTERVAL            = cmd_parser.get<useconds_t>("update-interval");
    PURCHASE_STREAM_INTERVAL   = cmd_parser.get<useconds_t>("purchase-interval");
    AGGREGATION_WINDOW_WIDTH   = cmd_parser.get<int>("aggregation-window-width");
    PURCHASE_COUNT             = cmd_parser.get<int>("purchase-count");
    GOODS_COUNT                = cmd_parser.get<int>("goods-count");
    MAX_PRICE                  = cmd_parser.get<int>("max-price");
    MIN_PRICE                  = cmd_parser.get<int>("min-price");
    SELECTION_CONDITION        = cmd_parser.get<std::string>("selection-condition");
    PURCHASE_SCHEMA_DEFINITION = cmd_parser.get<std::string>("purchase-schema");
    GOODS_SCHEMA_DEFINITION    = cmd_parser.get<std::string>("goods-schema");
}

void parse_option(cmdline::parser& cmd_parser, int argc, char** argv)
{
    // add specified type of variable.
    // 1st argument is long name
    // 2nd argument is short name (no short name if '\0' specified)
    // 3rd argument is description
    // 4th argument is mandatory (optional. default is false)
    // 5th argument is default value  (optional. it used when mandatory is false)

    // cmd_parser.add<std::string>("relation-file", '\0', "Relation file name", false, "");

    // Parameter
    cmd_parser.add<useconds_t>("update-interval", '\0', "update interval", false, 1000);
    cmd_parser.add<useconds_t>("purchase-interval", '\0', "Purchase interval", false, 1000);
    cmd_parser.add<int>("aggregation-window-width", '\0', "window width for aggregation", false, 10);
    cmd_parser.add<int>("purchase-count", '\0', "Purchase count", false, 1000);
    cmd_parser.add<int>("goods-count", '\0', "Goods count", false, 1000);
    cmd_parser.add<int>("max-price", '\0', "Max price for purchases", false, 100000);
    cmd_parser.add<int>("min-price", '\0', "Min price for purchases", false, 1000);

    // Condition
    cmd_parser.add<std::string>("selection-condition", '\0', "Condition for selection", false, "PRICE < 5000");

    // Schema definition
    cmd_parser.add<std::string>("purchase-schema", '\0', "Purchase schema", false, "CREATE STREAM PURCHASES(GOODS_ID INT, USER_ID INT)");
    cmd_parser.add<std::string>("goods-schema", '\0', "Goods schema", false, "CREATE TABLE GOODS(ID INT, PRICE INT)");

    // Finish!
    cmd_parser.parse_check(argc, argv);
}

#include <sys/time.h>

double get_current_time_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) * 0.001 * 0.001;
}

int main(int argc, char **argv)
{
    using namespace currentia;

    // create a parser
    cmdline::parser cmd_parser;
    parse_option(cmd_parser, argc, argv);
    set_parameters_from_option(cmd_parser);

    initialize(cmd_parser);

    typedef void* (*pthread_body_t)(void*);
    pthread_t listen_thread, process_thread, stream_sending_thread;

    double begin_time = get_current_time_in_seconds();

    pthread_create(&listen_thread, NULL, reinterpret_cast<pthread_body_t>(update_status_thread_body), NULL);
    pthread_create(&process_thread, NULL, reinterpret_cast<pthread_body_t>(process_stream_thread_body), NULL);
    pthread_create(&stream_sending_thread, NULL, reinterpret_cast<pthread_body_t>(stream_sending_thread_body), NULL);

    pthread_join(listen_thread, NULL);
    pthread_join(process_thread, NULL);
    pthread_join(stream_sending_thread, NULL);

    double end_time = get_current_time_in_seconds();
    double elapsed_seconds = end_time - begin_time;
    double throughput = PURCHASE_COUNT / elapsed_seconds;

    std::cerr << "Finished processing " << PURCHASE_COUNT << " tuples in " << elapsed_seconds << " secs." << std::endl
              << "Throughput: " << throughput << " tps" << std::endl;

    return 0;
}
