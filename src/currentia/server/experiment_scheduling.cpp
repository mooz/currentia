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

#include "currentia/core/operator/operator-visitor-visualize-dot.h"

#include "currentia/server/stream-sender.h"
#include "currentia/server/stream-consumer.h"
#include "currentia/server/relation-updater.h"
#include "currentia/server/query-processor.h"

#include "currentia/util/time.h"
#include "currentia/util/ansi-color.h"

#include <thread>
#include <iostream>
#include <fstream>
#include <cstdlib>

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

            if (!CommitOperatorFinder::find_commit_operator(query_ptr.get())) {
                // When commit operator isn't available in the plan,
                // don't enable not concurrency control mode
                cc_method = "none";
            }

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

        uint32_t xor128(void) {
          static uint32_t x = 123456789;
          static uint32_t y = 362436069;
          static uint32_t z = 521288629;
          static uint32_t w = 88675123;
          uint32_t t;
          t = x ^ (x << 11);
          x = y; y = z; z = w;
          return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
        }

        int generate_goods_price(int goods_id) {
            return xor128() % 10000 + 100;
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
            useconds_t update_duration = cmd_parser_.get<useconds_t>("update-duration");

            auto relation = query_container_->get_relation_by_name("R");
            insert_tuples_to_relation(relation, total_events);

            auto query_ptr = query_container_->get_root_operator_by_stream_name("ResultStream");
            std::shared_ptr<AbstractScheduler> scheduler(create_scheduler(query_ptr));

            if (cmd_parser_.exist("efficient-scheduling")) {
                scheduler->set_efficient_scheduling_enabled(true);
            }

            if (cmd_parser_.exist("output-dot")) {
                std::ofstream dot_ofs("/tmp/query_tree.dot", std::ios::out | std::ios::trunc);
                OperatorVisualizeDot::output_tree_as_dot(query_ptr, dot_ofs);
                dot_ofs.close();
                if (::system("dot -Tpng /tmp/query_tree.dot -o /tmp/query_tree.png && pkill eog ; eog /tmp/query_tree.png &")) {
                    std::cerr << "Failed to generate dot file" << std::endl;
                }
            }

            StreamConsumer stream_consumer(query_container_->get_stream_by_name("ResultStream"));
            RelationUpdater relation_updater(query_container_->get_relation_by_name("R"), update_interval, update_duration);
            QueryProcessor query_processor(scheduler);

            // First, insert whole streams
            ConcreteStreamSender stream_sender(query_container_->get_adapter_input_stream_by_name("InputStream"), total_events);
            stream_sender.start();
            stream_sender.wait();

            ConcreteStreamSender stream_sender2(query_container_->get_adapter_input_stream_by_name("InputStream2"), total_events);
            stream_sender2.start();
            stream_sender2.wait();

            std::cout << "Finished inserting" << std::endl;

            stream_consumer.start();
            relation_updater.start();
            TIME_IT(elapsed_seconds) {
                query_processor.start();
                stream_consumer.wait(); // wait for whole results
            }

            relation_updater.stop();
            query_processor.stop_and_wait();

            result_ios << "OK, finished loop" << std::endl;

            double throughput_query = total_events / elapsed_seconds;
            double throughput_update = relation_updater.get_update_count() / elapsed_seconds;

#define OUTPUT_ENTRY(key, value)                                        \
            if (cmd_parser_.exist("no-color")) {                        \
                result_ios << key << ": " << value << std::endl;        \
            } else {                                                    \
                result_ios << ansi::bold << ansi::cyan << key << ansi::reset \
                           << ": " << ansi::bold << ansi::yellow << value << ansi::reset << std::endl; \
            }

            OUTPUT_ENTRY("Efficient Scheduling", scheduler->efficient_scheduling_enabled());
            OUTPUT_ENTRY("Events", total_events);
            OUTPUT_ENTRY("Elapsed", elapsed_seconds << " secs");
            OUTPUT_ENTRY("Update Rate", 1.0 / time::usec_to_sec(update_interval) << " qps");
            OUTPUT_ENTRY("Query Throughput", throughput_query << " tps");
            OUTPUT_ENTRY("Update Throughput", throughput_update << " qps");

            OUTPUT_ENTRY("Scheduler Batch Process Count", cmd_parser_.get<int>("max-events-n-consume") << " tuples");

            if (auto acc = std::dynamic_pointer_cast<AbstractCCScheduler>(scheduler)) {
                OUTPUT_ENTRY("Redo", acc->get_redo_counts() << " times");
                auto commit_op = dynamic_cast<OperatorMean*>(acc->get_commit_operator());
                if (commit_op) {
                    OUTPUT_ENTRY("Consistent Rate", commit_op->get_consistent_rate());
                    OUTPUT_ENTRY("Window", commit_op->get_window().toString());
                }

#ifdef CURRENTIA_CHECK_STATISTICS
                OUTPUT_ENTRY("# of Reset Tuples", acc->get_reset_tuples_count());
                OUTPUT_ENTRY("# of Operator Evaluation Count", acc->get_total_evaluation_count());
#endif
            }

            OUTPUT_ENTRY("Method", cmd_parser_.get<std::string>("method"));

            // Selectivity
#ifdef CURRENTIA_CHECK_STATISTICS
            auto operators = OperatorVisitorSerializer::serialize_tree(query_ptr);
            for (auto iter = operators.begin(), iter_end = operators.end();
                 iter != iter_end; ++iter) {
                auto op = (*iter);
                double selectivity = op->get_selectivity();
                OUTPUT_ENTRY("Selectivity" << op->get_name(), selectivity);
            }
#endif

#ifdef CURRENTIA_DEBUG
            std::cout << "Leave run method!" << std::endl;
#endif

            exit(0);
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

    cmd_parser.add("output-dot", '\0', "Output operator tree as a dot file");
    cmd_parser.add("no-color", '\0', "Suppress colored output");
    cmd_parser.add("efficient-scheduling", '\0', "Enable efficient scheduling mode (constraint)");

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
        std::cerr << "Failed: " << error << std::endl;
        return 1;
    } catch (char* error) {
        std::cerr << "Failed: " << error << std::endl;
        return 1;
    } catch (const char* error) {
        std::cerr << "Failed: " << error << std::endl;
        return 1;
    }
    // catch (...) {
    //     std::cerr << "Failed: Unknown Error" << std::endl;
    //     return 1;
    // }

    return 0;
}
