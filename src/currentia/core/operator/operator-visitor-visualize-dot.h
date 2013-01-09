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
        std::vector<Operator*> operator_stack_;
        std::map<std::string, int> duplication_count_map_;
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

        void push_operator(Operator* op) {
            operator_stack_.push_back(op);

            std::string op_name = op->get_name();
            if (duplication_count_map_.find(op_name) == duplication_count_map_.end()) {
                duplication_count_map_[op_name] = 0;
            } else {
                duplication_count_map_[op_name] = duplication_count_map_[op_name] + 1;
            }
        }

        void pop_operator() {
            operator_stack_.pop_back();
        }

        void visit(SingleInputOperator* op) {
            push_operator(op);
            dispatch(op->get_parent_operator().get());
            pop_operator();
        }

        void visit(DoubleInputOperator* op) {
            push_operator(op);
            dispatch(op->get_parent_left_operator().get());
            dispatch(op->get_parent_right_operator().get());
            pop_operator();
        }

        void visit(OperatorStreamAdapter* op) {
            push_operator(op);
            // Leaf node. Output current stack.
            print_operator_stack();
            pop_operator();
        }

        void print_operator_stack() {
            auto iter = operator_stack_.rbegin();
            auto iter_end = operator_stack_.rend();
            dot_output_ << "  ";
            for (; iter != iter_end;) {
                std::string op_name = (*iter)->get_name();
                dot_output_ << boost::algorithm::replace_all_copy(op_name, "-", "");
                int duplication_count = duplication_count_map_[op_name];
                if (duplication_count > 0) {
                    dot_output_ << duplication_count + 1;
                }

                if (++iter == iter_end)
                    break;
                dot_output_ << " -> ";
            }
            dot_output_ << ";" << std::endl;
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
