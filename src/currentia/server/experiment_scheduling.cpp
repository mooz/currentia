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

#include "currentia/server/stream-sender.h"
#include "currentia/server/relation-updater.h"
#include "currentia/server/query-processor.h"

#include "currentia/util/time.h"

#include <thread>
#include <iostream>

namespace currentia {
    class ConcreteStreamSender : public StreamSender {
    public:
        ConcreteStreamSender(const Stream::ptr_t& stream,
                             long total_tuples,
                             long send_interval = 0):
            StreamSender(stream, total_tuples, send_interval) {
            srand(1);
        }

    private:
        Tuple::ptr_t get_next(long id) {
            return Tuple::create_easy(
                stream_->get_schema(),
                id,
                "abcdefghijklmnop",
                get_next_age()
            );
        }

        int get_next_age() {
            return (rand() % 100);
        }
    };

    class ExperimentScheduling {
        CPLQueryContainer::ptr_t query_container_;
        cmdline::parser cmd_parser_;

    public:
        ExperimentScheduling(std::istream& istream,
                             cmdline::parser& cmd_parser):
            cmd_parser_(cmd_parser) {
            query_container_ = parse_cpl(&istream);
        }

        AbstractScheduler* create_scheduler(const Operator::ptr_t& query_ptr) {
            std::string cc_method = cmd_parser_.get<std::string>("method");
            int txn_joint_count = cmd_parser_.get<int>("txn-joint-count");

            SchedulingPolicyFactory::ptr_t scheduling_policy_factory(new RoundRobinPolicyFactory());

            AbstractScheduler* scheduler = NULL;
            if (cc_method == "optimistic")
                scheduler = new OptimisticCCScheduler(query_ptr, scheduling_policy_factory);
            else if (cc_method == "2pl")
                scheduler = new LockCCScheduler(query_ptr, scheduling_policy_factory, txn_joint_count);
            else if (cc_method == "snapshot")
                scheduler = new SnapshotCCScheduler(query_ptr, scheduling_policy_factory, txn_joint_count);
            else
                scheduler = new WithoutCCScheduler(query_ptr, scheduling_policy_factory);

            scheduler->set_batch_count(cmd_parser_.get<int>("max-events-n-consume"));

            return scheduler;
        }

        int generate_goods_price(int goods_id) {
            return 10000;
        }

        void insert_tuples_to_relation(Relation::ptr_t relation, int count) {
            Schema::ptr_t schema = relation->get_schema();
            for (int tuple_id = 0; tuple_id < count; ++tuple_id) {
                relation->insert(
                    Tuple::create_easy(schema, tuple_id, generate_goods_price(tuple_id))
                );
            }
        }

        void run(std::ostream& result_ios = std::cout) {
            long total_events = cmd_parser_.get<int>("total-events");
            useconds_t update_interval = cmd_parser_.get<useconds_t>("update-interval");

            auto query_ptr = query_container_->get_root_operator_by_stream_name("ResultStream");
            auto relation = query_container_->get_relation_by_name("R");
            insert_tuples_to_relation(relation, total_events);

            AbstractScheduler* scheduler = create_scheduler(query_ptr);

            auto stream_sender = ConcreteStreamSender(query_container_->get_stream_by_name("InputStream"), total_events);
            auto relation_updater = RelationUpdater(query_container_->get_relation_by_name("R"), update_interval);
            auto query_processor = QueryProcessor(scheduler);

            stream_sender.run(); // First, dispatch every events and wait for them

            TIME_IT(elapsed_seconds) {
                // query processing / relation updating
                std::thread relation_update_thread = relation_updater.create_thread();
                // query processing / relation updating
                std::thread query_processing_thread = query_processor.create_thread();

                query_processing_thread.join();
                relation_updater.stop();
            }

            // Display results
#ifdef CURRENTIA_DEBUG
            auto result_stream = query_container_->get_stream_by_name("ResultStream");
            while (true) {
                Tuple::ptr_t next_tuple = result_stream->dequeue();
                if (next_tuple->is_eos())
                    break;
                std::cout << next_tuple->toString() << std::endl;
            }
#endif

            double throughput_query = total_events / elapsed_seconds;
            double throughput_update = relation_updater.get_update_count() / elapsed_seconds;

            result_ios
                << "Events: " << total_events << std::endl
                << "Elapsed: " << elapsed_seconds << " secs" << std::endl
                << "Update Rate: " << 1.0 / time::usec_to_sec(update_interval) << " qps" << std::endl
                << "Query Throughput: " << throughput_query << " tps" << std::endl
                << "Update Throughput: " << throughput_update << " qps" << std::endl;

            if (OptimisticCCScheduler* occ = dynamic_cast<OptimisticCCScheduler*>(scheduler)) {
                std::clog << "Redo: " << occ->get_redo_counts() << " times" << std::endl;
            }
            if (AbstractCCScheduler* acc = dynamic_cast<AbstractCCScheduler*>(scheduler)) {
                std::clog << "Consistent Rate: " << acc->get_consistent_rate() << std::endl;
                TraitAggregationOperator* op = dynamic_cast<OperatorMean*>(acc->get_commit_operator());
                std::clog << "Window: " << op->get_window().toString() << std::endl;
            }

            result_ios << "Method: " << cmd_parser_.get<std::string>("method") << std::endl;
        }
    };
}

void parse_option(cmdline::parser& cmd_parser, int argc, char** argv)
{
    cmd_parser.add<std::string>("method", '\0', "consistency preserving method", false, "none",
                                cmdline::oneof<std::string>("optimistic", "2pl", "snapshot", "none"));

    cmd_parser.add<int>("txn-joint-count", '\0', "Joint count for txn", false, 1);

    cmd_parser.add<int>("max-events-n-consume", '\0', "Maximum number of events to be evaluated at once", false, 1);

    cmd_parser.add<useconds_t>("update-interval", '\0', "update interval", false, 1000);
    cmd_parser.add<useconds_t>("update-duration", '\0', "time needed to update a relation", false, 10);
    cmd_parser.add<int>("total-events", '\0', "Events count", false, 1000);

    cmd_parser.parse_check(argc, argv);
}

int main(int argc, char **argv)
{
    cmdline::parser cmd_parser;
    parse_option(cmd_parser, argc, argv);

    try {
        auto experiment = currentia::ExperimentScheduling(std::cin, cmd_parser);
        experiment.run();
    } catch (const std::string& error) {
        std::cerr << "Failed to initialize: " << error << std::endl;
        return 1;
    }

    return 0;
}
