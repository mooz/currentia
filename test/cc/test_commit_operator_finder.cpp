#include <gtest/gtest.h>

#include "currentia/core/cc/commit-operator-finder.h"
#include "currentia/query/cpl-parse.h"

using namespace currentia;

TEST (TestCommitOperatorFinder, test_commit_operator) {
    std::stringstream ss(
        "relation R(r: int)\n"
        "\n"
        "stream A(a: int)\n"
        "stream B(b: int)\n"
        "stream C(c: int)\n"
        "stream D(d: int)\n"
        "stream E(e: int)\n"
        "\n"
        "stream B_R from B { combine R where B.b = R.r }\n"
        "stream A_B_R from A[1], B_R[1] where A.a = B.b\n"
        "\n"
        "stream C_R from C { combine R where C.c = R.r }\n"
        "stream D_E from D[1], E[1] where D.d = E.e\n"
        "stream C_R_D_E from C_R[1], D_E[1] where C_R.c = D_E.e\n"
        "\n"
        "stream final_join from A_B_R[1], C_R_D_E[1] where A.a = C.c\n"
        "\n"
        "stream result from final_join\n"
        "{\n"
        "  mean final_join.a [5]\n"
        "  select final_join.c > 200\n"
        "}"
    );

    auto query_container = parse_cpl(&ss);
    auto root_operator = query_container->get_root_operator_by_stream_name("result");

    CommitOperatorFinder finder(root_operator.get());

    auto select_operator = dynamic_cast<SingleInputOperator*>(query_container->get_root_operator_by_stream_name("result").get());
    auto mean_operator = dynamic_cast<SingleInputOperator*>(select_operator->get_parent_operator().get());

    EXPECT_EQ(mean_operator, finder.get_commit_operator());
}

TEST (TestCommitOperatorFinder, test_commit_operator_none) {
    std::stringstream ss(
        "relation R(r: int)\n"
        "stream A(a: int)\n"
        "stream result from A { combine R where R.r > A.a }"
    );

    auto query_container = parse_cpl(&ss);
    auto root_operator = query_container->get_root_operator_by_stream_name("result");

    CommitOperatorFinder finder(root_operator.get());

    EXPECT_EQ(NULL, finder.get_commit_operator());
}

TEST (TestCommitOperatorFinder, test_commit_operator_aggregate) {
    std::stringstream ss(
        "relation R(r: int)\n"
        "stream A(a: int)\n"
        "stream result from A { combine R where R.r > A.a mean A.a [10] }"
    );

    auto query_container = parse_cpl(&ss);
    auto root_operator = query_container->get_root_operator_by_stream_name("result");

    CommitOperatorFinder finder(root_operator.get());

    auto mean_operator = query_container->get_root_operator_by_stream_name("result").get();

    EXPECT_EQ(mean_operator, finder.get_commit_operator());
}
