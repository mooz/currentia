// -*- c++ -*-

#include "currentia/core/operator/condition.h"
#include "currentia/core/operator/operator-election.h"
#include "currentia/core/operator/operator-selection.h"
#include "currentia/core/operator/operator-simple-relation-join.h"
#include "currentia/core/relation.h"

#include "currentia/core/scheduler/policy/scheduling-policy.h"
#include "currentia/core/scheduler/policy/scheduling-policy-round-robin.h"

#include "currentia/core/scheduler/abstract-scheduler.h"
#include "currentia/core/cc/without-cc-scheduler.h"
#include "currentia/core/cc/optimistic-cc-scheduler.h"
#include "currentia/core/cc/lock-cc-scheduler.h"
#include "currentia/core/cc/snapshot-cc-scheduler.h"

#include "currentia/core/stream.h"
#include "currentia/core/thread.h"
#include "thirdparty/cmdline.h"
#include "currentia/query/cpl-parse.h"

#include <boost/thread.hpp>
#include <sys/time.h>
#include <iostream>

using namespace currentia;

#define LOG(x) std::cout << x << std::endl

double get_current_time_in_seconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return static_cast<double>(tv.tv_sec) + static_cast<double>(tv.tv_usec) * 0.001 * 0.001;
}

static Stream::ptr_t purchase_stream;
static Relation::ptr_t goods_relation;

Operator::ptr_t query_ptr;
static Stream::ptr_t result_stream;

// ============================================================
// Begin Helpers
// ======================================================== {{{

double interval_to_rate(useconds_t interval)
{
    return 1.0 / (static_cast<double>(interval) * 0.001 * 0.001);
}

// }}} ========================================================
// End Helpers
// ============================================================

// ============================================================
// Begin Body
// ======================================================== {{{

double begin_time;
double end_time;

// Parameters
static int PURCHASE_COUNT;
static int GOODS_COUNT;

static useconds_t UPDATE_INTERVAL;
static useconds_t UPDATE_TIME;

static int updated_status_count;
void* update_status_thread_body(void* argument)
{
    updated_status_count = 0;

    while (true) {
        // randomly select a tuple and update it
        if (UPDATE_INTERVAL > 0)
            usleep(UPDATE_INTERVAL);

        // std::clog << "UPDATE: try to lock" << std::endl;
        goods_relation->read_write_lock();
        // std::clog << "UPDATE: updated" << std::endl;
        goods_relation->update();
        usleep(UPDATE_TIME);
        goods_relation->unlock();

        updated_status_count++;
    }

    return NULL;
}

void* consume_output_stream_thread_body(void* argument)
{
    try {
        while (true) {
            Tuple::ptr_t next_tuple = result_stream->dequeue();
            if (next_tuple->is_eos())
                break;
        }
    } catch (const char* error_message) {
        std::cerr << "Error while consuming output stream: " << error_message << std::endl;
    }

    end_time = get_current_time_in_seconds();

    return NULL;
}

AbstractScheduler *scheduler;
void* process_stream_thread_body(void* argument)
{
    try {
        while (true) {
            scheduler->wake_up();
            thread::scheduler_yield();
        }
    } catch (const char* error_message) {
        std::cerr << "Error while processing stream: " << error_message << std::endl;
    }

    return NULL;
}

// Send purchase data
static useconds_t PURCHASE_STREAM_INTERVAL;
void* stream_sending_thread_body(void* argument)
{
    try {
        begin_time = get_current_time_in_seconds();

        Schema::ptr_t schema_ptr = purchase_stream->get_schema();
        std::clog << "purchase_stream: " << purchase_stream << std::endl;

        for (int i = 0; i < PURCHASE_COUNT; ++i) {
            purchase_stream->enqueue(
                Tuple::create_easy(schema_ptr, i % GOODS_COUNT, rand())
            );
            if (PURCHASE_STREAM_INTERVAL > 0)
                usleep(PURCHASE_STREAM_INTERVAL);
        }

        // std::cout << "------------------------------------------------------------" << std::endl;
        // std::cout << "Sent EOS !!!" << std::endl;
        // std::cout << "------------------------------------------------------------" << std::endl;

        std::clog << "Sent finish!!!!!!!!!!!!!!" << std::endl;
        purchase_stream->enqueue(Tuple::create_eos()); // Finish!

    } catch (const char* error_message) {
        std::cerr << "Error while sending data stream: " << error_message << std::endl;
    }

    return NULL;
}

// }}} ========================================================
// End Body
// ============================================================

// ============================================================
// Goods
// ======================================================== {{{

static int MIN_PRICE;
static int MAX_PRICE;
int generate_goods_price(int id)
{
    return MIN_PRICE + rand() % (MAX_PRICE - MIN_PRICE);
}

void insert_goods(Relation::ptr_t relation)
{
    Schema::ptr_t schema = relation->get_schema();
    for (int goods_id = 0; goods_id < GOODS_COUNT; ++goods_id) {
        relation->insert(
            Tuple::create_easy(schema, goods_id, generate_goods_price(goods_id))
        );
    }
}

// }}} ========================================================
// Goods
// ============================================================

// }}} ========================================================
// Begin ~ End : Versioning
// ============================================================

void initialize(const cmdline::parser& cmd_parser)
{
    srand(1);

    std::cout << "Input query:" << std::endl;
    CPLQueryContainer::ptr_t parse_result = parse_cpl(&std::cin);

    // Setup input stream
    purchase_stream = parse_result->get_input_stream_by_name("purchases");

    // Setup relation
    goods_relation = parse_result->get_relation_by_name("goods");
    insert_goods(goods_relation);

    // Record query and result
    result_stream = parse_result->get_stream_by_name("result");
    query_ptr = parse_result->get_root_operator_for_stream(result_stream);

    SchedulingPolicyFactory::ptr_t scheduling_policy_factory(new RoundRobinPolicyFactory());

    std::string cc_method = cmd_parser.get<std::string>("method");
    int txn_joint_count = cmd_parser.get<int>("txn-joint-count");
    if (cc_method == "optimistic")
        scheduler = new OptimisticCCScheduler(query_ptr, scheduling_policy_factory);
    else if (cc_method == "2pl")
        scheduler = new LockCCScheduler(query_ptr, scheduling_policy_factory, txn_joint_count);
    else if (cc_method == "snapshot")
        scheduler = new SnapshotCCScheduler(query_ptr, scheduling_policy_factory, txn_joint_count);
    else
        scheduler = new WithoutCCScheduler(query_ptr, scheduling_policy_factory);
}

void set_parameters_from_option(cmdline::parser& cmd_parser)
{
    UPDATE_INTERVAL            = cmd_parser.get<useconds_t>("update-interval");
    UPDATE_TIME                = cmd_parser.get<useconds_t>("update-time");
    PURCHASE_STREAM_INTERVAL   = cmd_parser.get<useconds_t>("purchase-interval");
    PURCHASE_COUNT             = cmd_parser.get<int>("purchase-count");
    GOODS_COUNT                = cmd_parser.get<int>("goods-count");
    MAX_PRICE                  = cmd_parser.get<int>("max-price");
    MIN_PRICE                  = cmd_parser.get<int>("min-price");
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
    cmd_parser.add<std::string>("method", '\0', "consistency preserving method", false, "none",
                                cmdline::oneof<std::string>("optimistic", "2pl", "snapshot", "none"));
    cmd_parser.add<int>("txn-joint-count", '\0', "Joint count for txn", false, 1);

    cmd_parser.add<useconds_t>("update-interval", '\0', "update interval", false, 1000);
    cmd_parser.add<useconds_t>("update-time", '\0', "time needed to update a relation", false, 10);
    cmd_parser.add<useconds_t>("purchase-interval", '\0', "Purchase interval", false, 1000);
    cmd_parser.add<int>("purchase-count", '\0', "Purchase count", false, 1000);
    cmd_parser.add<int>("goods-count", '\0', "Goods count", false, 1000);
    cmd_parser.add<int>("max-price", '\0', "Max price for purchases", false, 100000);
    cmd_parser.add<int>("min-price", '\0', "Min price for purchases", false, 1000);

    cmd_parser.add<bool>("reuse-snapshot", '\0', "Reuse snapshot when possible", false, true);

    // Finish!
    cmd_parser.parse_check(argc, argv);
}

class StreamSender {
protected:
    Stream::ptr_t stream_;
    Schema::ptr_t schema_;

    long send_count_;
    long send_interval_;
    long total_tuples_;

public:
    StreamSender(const Stream::ptr_t& stream,
                 long total_tuples,
                 long send_interval_):
        stream_(stream),
        schema_(stream->get_schema()),
        send_count_(0),
        send_interval_(0),
        total_tuples_(total_tuples_) {
    }

    virtual ~StreamSender() = 0;

    void run() {
        // boost::thread thread(&StreamSender::run, this);
        for (int i = 0; i < total_tuples_; ++i) {
            stream_->enqueue(get_next(i));
            if (send_interval_ > 0)
                usleep(send_interval_);
        }
        purchase_stream->enqueue(Tuple::create_eos());
    }

    virtual Tuple::ptr_t get_next(long i) {
        return Tuple::create_easy(schema_, i % GOODS_COUNT, rand());
    }
};
StreamSender::~StreamSender() {}

class RelationUpdater {
    Relation::ptr_t relation_;

    long update_count_;
    long update_interval_;

public:
    RelationUpdater(const Relation::ptr_t& relation):
        relation_(relation),
        update_count_(0),
        update_interval_(0) {
    }

    void run() {
        while (true) {
            // randomly select a tuple and update it
            if (update_interval_ > 0)
                usleep(update_interval_);

            relation_->read_write_lock();
            relation_->update();
            usleep(update_interval_);
            relation_->unlock();

            update_count_++;
        }
    }
};

class PurchaseSender : public StreamSender {
    int goods_count_;

public:
    PurchaseSender(const Stream::ptr_t& stream,
                   long total_tuples,
                   long send_interval,
                   long goods_count):
        StreamSender(stream, total_tuples, send_interval),
        goods_count_(goods_count) {
    }

    Tuple::ptr_t get_next(long i) {
        return Tuple::create_easy(schema_, i % goods_count_, rand());
    }
};

int main(int argc, char **argv)
{
    using namespace currentia;

    // create a parser
    cmdline::parser cmd_parser;
    try {
        parse_option(cmd_parser, argc, argv);
        set_parameters_from_option(cmd_parser);
    } catch (const std::string& error) {
        std::cerr << error << std::endl;
    } catch (const char* error) {
        std::cerr << error << std::endl;
    }

    try {
        initialize(cmd_parser);
    } catch (const std::string& error) {
        std::cerr << "Failed to initialize: " << error << std::endl;
        return 1;
    }

    typedef void* (*pthread_body_t)(void*);
    pthread_t update_status_thread, process_stream_thread, consume_output_stream_thread, stream_sending_thread;

    pthread_create(&update_status_thread, NULL, reinterpret_cast<pthread_body_t>(update_status_thread_body), NULL);
    pthread_create(&process_stream_thread, NULL, reinterpret_cast<pthread_body_t>(process_stream_thread_body), NULL);
    pthread_create(&consume_output_stream_thread, NULL, reinterpret_cast<pthread_body_t>(consume_output_stream_thread_body), NULL);
    pthread_create(&stream_sending_thread, NULL, reinterpret_cast<pthread_body_t>(stream_sending_thread_body), NULL);

    pthread_join(consume_output_stream_thread, NULL);
    pthread_join(stream_sending_thread, NULL);

    // pthread_cancel(process_stream_thread);
    // pthread_cancel(update_status_thread);

    double elapsed_seconds = end_time - begin_time;
    double throughput_query = PURCHASE_COUNT / elapsed_seconds;
    double throughput_update = updated_status_count / elapsed_seconds;

    std::clog
        << "Tuples: " << PURCHASE_COUNT << std::endl
        << "Elapsed: " << elapsed_seconds << " secs" << std::endl
        << "Stream Rate: " << interval_to_rate(PURCHASE_STREAM_INTERVAL) << " tps" << std::endl
        << "Update Rate: " << interval_to_rate(UPDATE_INTERVAL) << " qps" << std::endl
        << "Query Throughput: " << throughput_query << " tps" << std::endl
        << "Update Throughput: " << throughput_update << " qps" << std::endl;

    if (OptimisticCCScheduler* occ = dynamic_cast<OptimisticCCScheduler*>(scheduler)) {
        std::clog << "Redo: " << occ->get_redo_counts() << " times" << std::endl;
    }
    if (AbstractCCScheduler* acc = dynamic_cast<AbstractCCScheduler*>(scheduler)) {
        std::clog << "Consistent Rate: " << acc->get_consistent_rate() << std::endl;
        TraitAggregationOperator* op = dynamic_cast<OperatorMean*>(acc->get_commit_operator());
        std::clog << "Window: " << op->get_window() << std::endl;
    }
    // TODO: Create common ancestor class for AbstractCCScheduler and WithoutCCScheduler
    if (WithoutCCScheduler* rcc = dynamic_cast<WithoutCCScheduler*>(scheduler)) {
        std::clog << "Consistent Rate: " << rcc->get_consistent_rate() << std::endl;
        TraitAggregationOperator* op = dynamic_cast<OperatorMean*>(rcc->get_commit_operator());
        std::clog << "Window: " << op->get_window() << std::endl;
    }

    std::clog << "Method: " << cmd_parser.get<std::string>("method") << std::endl;

    // << "Selectivity: " << global_selection->get_selectivity() << std::endl
    // << "Window: " << AGGREGATION_WINDOW_WIDTH << std::endl;

    return 0;
}
