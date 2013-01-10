// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_VISITOR_VISUALIZE_DOT_H_
#define CURRENTIA_OPERATOR_VISITOR_VISUALIZE_DOT_H_

#include <iostream>

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-abstract-visitor.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/pointer.h"

#include "currentia/util/print.h"

#include <boost/algorithm/string.hpp>

namespace currentia {
    // Serialize a operator tree by topological sort algorithm
    class OperatorVisualizeDot : public OperatorAbstractVisitor,
                                 public Pointable<OperatorVisualizeDot> {
        std::map<std::string, int> duplication_count_map_;
        std::map<Operator*, int> operator_id_map_;
        std::ostream& dot_output_;

    public:
        typedef Pointable<OperatorVisualizeDot>::ptr_t ptr_t;

        OperatorVisualizeDot(std::ostream& dot_output):
            dot_output_(dot_output) {
        }

        static
        void output_tree_as_dot(const Operator::ptr_t& root_operator_ptr,
                                std::ostream& dot_output = std::cout,
                                std::string query_name = "Query") {
            OperatorVisualizeDot dotter(dot_output);
            dot_output << "digraph " << query_name << " {" << std::endl;
            dotter.dispatch(root_operator_ptr.get());
            dot_output << "}" << std::endl;
        }

        // Push / Pop

        void record_operator(Operator* op) {
            std::string op_name = op->get_name();
            if (duplication_count_map_.find(op_name) == duplication_count_map_.end()) {
                duplication_count_map_[op_name] = 0;
            } else {
                duplication_count_map_[op_name] = duplication_count_map_[op_name] + 1;
            }
            operator_id_map_[op] = duplication_count_map_[op_name];

            if (op->is_commit_operator()) {
                dot_output_ << "  " << get_node_name_from_operator(op) << " [style=\"filled\",color=\"yellow\"];" << std::endl;
            }
        }

        void connect_edge(Operator* from, Operator* to) {
            dot_output_ << "  "
                        << get_node_name_from_operator(from)
                        << " -> "
                        << get_node_name_from_operator(to);

            if (from->get_output_stream()->is_backed_up()) {
                dot_output_ << " [label=\"Has Backup\"]" << std::endl;
            }

            dot_output_ << ";" << std::endl;
        }

        // Print

        std::string get_node_name_from_operator(Operator* op) {
            std::stringstream ss;

            ss << boost::algorithm::replace_all_copy(op->get_name(), "-", "");
            int id = operator_id_map_[op];
            ss << id + 1;

            return ss.str();
        }

        // Traversing

        void visit(SingleInputOperator* op) {
            record_operator(op);
            Operator* parent = op->get_parent_operator().get();
            dispatch(parent);

            connect_edge(parent, op);
        }

        void visit(DoubleInputOperator* op) {
            record_operator(op);
            Operator* left_parent = op->get_parent_left_operator().get();
            Operator* right_parent = op->get_parent_right_operator().get();
            dispatch(left_parent);
            dispatch(right_parent);

            connect_edge(left_parent, op);
            connect_edge(right_parent, op);
        }

        void visit(OperatorStreamAdapter* op) {
            record_operator(op);
        }

        CURRENTIA_DEFINE_OPERATOR_DISPATCHER();
    };
}

#endif  /* ! CURRENTIA_OPERATOR_VISITOR_VISUALIZE_DOT_H_ */
