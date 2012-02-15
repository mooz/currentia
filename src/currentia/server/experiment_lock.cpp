// -*- c++ -*-

#include "currentia/core/thread.h"
#include "currentia/server/server.h"

#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/operator-simple-relation-join.h"

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

Operator::ptr_t query_ptr;
Stream::ptr_t purchase_stream;
std::list<Tuple::ptr_t> goods_relation;

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

void* update_status_thread_body(void* argument)
{
    // update (throughput)

    return NULL;
}

// Request output of the query
static int PURCHASE_COUNT;
void* process_stream_thread_body(void* argument)
{
    LOG("Process Stream Begin");

    try {
        while (true) {
            Tuple::ptr_t next_tuple = query_ptr->next();
            if (next_tuple->is_eos())
                break;
            std::cout << next_tuple->toString() << std::endl;
        }
    } catch (const char* error_message) {
        std::cout << "Error while processing stream: " << error_message << std::endl;
    }

    LOG("Process Stream End");

    return NULL;
}

// Send purchase data
useconds_t PURCHASE_STREAM_INTERVAL;
void* stream_sending_thread_body(void* argument)
{
    LOG("Send Purchase Data Begin");

    Schema::ptr_t schema_ptr = purchase_stream->get_schema_ptr();

    for (int i = 0; i < PURCHASE_COUNT; ++i) {
        LOG("Send " << i << "th tuple");
        purchase_stream->enqueue(create_purchase_tuple(i /* goods id */, rand() /* user id */));
        usleep(PURCHASE_STREAM_INTERVAL);
    }

    purchase_stream->enqueue(Tuple::create_eos()); // Finish!

    LOG("Send Purchase Data End");

    return NULL;
}

// }}} ========================================================
// End Body
// ============================================================

// ============================================================
// Purchase
// ======================================================== {{{

std::string PURCHASE_SCHEMA_DEFINITION;
Schema::ptr_t purchase_schema_ptr;
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

std::string GOODS_SCHEMA_DEFINITION;
Schema::ptr_t goods_schema_ptr;
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

int MIN_PRICE;
int MAX_PRICE;
int generate_goods_price(int id)
{
    return MIN_PRICE + rand() % (MAX_PRICE - MIN_PRICE);
}

int GOODS_COUNT;
void set_goods_relation()
{
    create_goods_schema();

    for (int goods_id = 0; goods_id < GOODS_COUNT; ++goods_id) {
        goods_relation.push_back(create_goods_tuple(goods_id, generate_goods_price(goods_id)));
    }
    // std::ifstream relation_file;
    // relation_file.open();
}

// }}} ========================================================
// Goods
// ============================================================

void set_purchase_stream()
{
    create_purchase_schema();
    purchase_stream = create_stream_from_schema(purchase_schema_ptr);
}

std::string SELECTION_CONDITION;

void setup_query()
{
    LOG("Setup Query Begin!");

    Operator::ptr_t purchase_stream_current;

    Operator::ptr_t purchase_stream_adapter(new OperatorStreamAdapter(purchase_stream));
    {
        // Stream1 {{{
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
        // }}} Stream1

        purchase_stream_current = selection;
    }

    query_ptr = purchase_stream_current;

    LOG("Setup Query End!");
}

void initialize(cmdline::parser& cmd_parser)
{
    try {
        set_purchase_stream();
        set_goods_relation();
        // query
        setup_query();
    } catch (const char* error_message) {
        std::cerr << "Error: " << error_message << std::endl;
    }
}

void set_parameters_from_option(cmdline::parser& cmd_parser)
{
    PURCHASE_STREAM_INTERVAL   = cmd_parser.get<useconds_t>("purchase-interval");
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
    cmd_parser.add<useconds_t>("purchase-interval", '\0', "Purchase interval", false, 1000);
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

    pthread_create(&listen_thread, NULL, reinterpret_cast<pthread_body_t>(update_status_thread_body), NULL);
    pthread_create(&process_thread, NULL, reinterpret_cast<pthread_body_t>(process_stream_thread_body), NULL);
    pthread_create(&stream_sending_thread, NULL, reinterpret_cast<pthread_body_t>(stream_sending_thread_body), NULL);

    pthread_join(listen_thread, NULL);
    pthread_join(process_thread, NULL);
    pthread_join(stream_sending_thread, NULL);

    return 0;
}
