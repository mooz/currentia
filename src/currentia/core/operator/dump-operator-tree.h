// -*- c++ -*-

#ifndef CURRENTIA_OPERATOR_TREE_DUMPER_H_
#define CURRENTIA_OPERATOR_TREE_DUMPER_H_

#include "currentia/core/operator/double-input-operator.h"
#include "currentia/core/operator/operator-stream-adapter.h"
#include "currentia/core/operator/operator.h"
#include "currentia/core/operator/single-input-operator.h"
#include "currentia/core/operator/trait-aggregation-operator.h"
#include "currentia/core/operator/trait-resource-reference-operator.h"
#include "currentia/core/pointer.h"

#include <string>

namespace currentia {
    std::string dump_operator_tree(Operator* op) {
        std::stringstream ss;

        ss << op->get_name() << "<" << op << "> ";

        if (SingleInputOperator* single_op = dynamic_cast<SingleInputOperator*>(op)) {
            ss << "(input stream: ";
            ss << single_op->get_parent_operator()->get_output_stream();
            ss << ")";
            ss << std::endl;
            ss << dump_operator_tree(single_op->get_parent_operator().get());
        } else if (DoubleInputOperator* double_op = dynamic_cast<DoubleInputOperator*>(op)) {
            ss << "(left input stream: ";
            ss << double_op->get_parent_left_operator()->get_output_stream();
            ss << ", ";

            ss << "right input stream: ";
            ss << double_op->get_parent_right_operator()->get_output_stream();
            ss << ")";
            ss << std::endl;

            ss << dump_operator_tree(double_op->get_parent_left_operator().get());
            ss << dump_operator_tree(double_op->get_parent_right_operator().get());
        } else if (OperatorStreamAdapter* stream_op = dynamic_cast<OperatorStreamAdapter*>(op)) {
            ss << "(stream adapter: ";
            ss << stream_op->get_input_stream();
            ss << ")";
            ss << std::endl;
        }

        return ss.str();
    }
}

#endif  /* ! CURRENTIA_OPERATOR_TREE_DUMPER_H_ */

