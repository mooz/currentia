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
        std::list<Operator*> operator_stack_;
        std::list<Operator*> operator_list_;
        std::map<Operator*, Operator*> child_map_;
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
            dotter.prettify_graph();
            dot_output << "}" << std::endl;
        }

        // Push / Pop

        void push_operator(Operator* op) {
            operator_stack_.push_back(op);

            std::string op_name = op->get_name();
            if (duplication_count_map_.find(op_name) == duplication_count_map_.end()) {
                duplication_count_map_[op_name] = 0;
            } else {
                duplication_count_map_[op_name] = duplication_count_map_[op_name] + 1;
            }
            operator_id_map_[op] = duplication_count_map_[op_name];
        }

        void pop_operator() {
            Operator* op = operator_stack_.back();
            operator_stack_.pop_back();
            // Also, collect all operators
            operator_list_.push_back(op);
        }

        // Print

        std::string get_node_name_from_operator(Operator* op) {
            std::stringstream ss;

            ss << boost::algorithm::replace_all_copy(op->get_name(), "-", "");
            int id = operator_id_map_[op];
            ss << id + 1;

            return ss.str();
        }

        void print_operator_stack() {
            auto iter = operator_stack_.rbegin();
            auto iter_end = operator_stack_.rend();
            dot_output_ << "  ";
            for (; iter != iter_end;) {
                Operator* op = (*iter);
                dot_output_ << get_node_name_from_operator(op);
                if (++iter == iter_end)
                    break;
                dot_output_ << " -> ";
            }
            dot_output_ << ";" << std::endl;
        }

        void prettify_graph() {
            auto iter = operator_list_.begin();
            auto iter_end = operator_list_.end();
            for (; iter != iter_end; ++iter) {
                Operator* op = (*iter);
                if (op->is_commit_operator()) {
                    dot_output_ << "  " << get_node_name_from_operator(op) << " [style=\"filled\",color=\"yellow\"];" << std::endl;
                }
                if (op->get_output_stream()->is_backed_up() &&
                    child_map_.find(op) != child_map_.end()) {
                    dot_output_ << "  " << get_node_name_from_operator(op) << " -> "
                                << get_node_name_from_operator(child_map_[op]) << " [label=\"Backup\"];" << std::endl;
                }
            }
        }

        // Traversing

        void visit(SingleInputOperator* op) {
            push_operator(op);
            Operator* parent = op->get_parent_operator().get();
            dispatch(parent);
            child_map_[parent] = op;
            pop_operator();
        }

        void visit(DoubleInputOperator* op) {
            push_operator(op);
            Operator* left_parent = op->get_parent_left_operator().get();
            Operator* right_parent = op->get_parent_right_operator().get();
            dispatch(left_parent);
            dispatch(right_parent);
            child_map_[left_parent] = op;
            child_map_[right_parent] = op;
            pop_operator();
        }

        void visit(OperatorStreamAdapter* op) {
            push_operator(op);
            // Leaf node. Output current stack.
            print_operator_stack();
            pop_operator();
        }

        void dispatch(Operator* op) {
            if (dynamic_cast<SingleInputOperator*>(op)) {
                dynamic_cast<SingleInputOperator*>(op)->accept(this);
            } else if (dynamic_cast<DoubleInputOperator*>(op)) {
                dynamic_cast<DoubleInputOperator*>(op)->accept(this);
            } else if (dynamic_cast<OperatorStreamAdapter*>(op)) {
                dynamic_cast<OperatorStreamAdapter*>(op)->accept(this);
            } else {
                throw (std::string("Unhandled operator: ") + op->toString()).c_str();
            }
        }
    };
}

#endif  /* ! CURRENTIA_OPERATOR_VISITOR_VISUALIZE_DOT_H_ */
