#include <gtest/gtest.h>

#include "currentia/core/cc/redo-area.h"
#include "currentia/query/cpl-parse.h"
#include "currentia/util/log.h"

using namespace currentia;

TEST (TestRedoArea, test_redo_area_big) {
    std::clog.rdbuf(new system::Log("currentia"));

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
    RedoArea redo_area(root_operator.get());
    auto redo_streams = redo_area.get_redo_streams();

    EXPECT_EQ(4, redo_streams.size());

    EXPECT_EQ(query_container->get_stream_by_name("B"), redo_streams[0]);
    EXPECT_EQ(query_container->get_stream_by_name("A"), redo_streams[1]);
    EXPECT_EQ(query_container->get_stream_by_name("C"), redo_streams[2]);
    EXPECT_EQ(query_container->get_stream_by_name("D_E"), redo_streams[3]);
}

TEST (TestCommitOperatorFinder, test_redo_area_none) {
    std::stringstream ss(
        "relation R(r: int)\n"
        "stream A(a: int)\n"
        "stream result from A { combine R where R.r > A.a }"
    );

    auto query_container = parse_cpl(&ss);
    auto root_operator = query_container->get_root_operator_by_stream_name("result");
    RedoArea redo_area(root_operator.get());
    auto redo_streams = redo_area.get_redo_streams();

    EXPECT_EQ(0, redo_streams.size());
}

TEST (TestCommitOperatorFinder, test_redo_area_aggregate_only) {
    std::stringstream ss(
        "relation R(r: int)\n"
        "stream A(a: int)\n"
        "stream result from A { combine R where R.r > A.a mean A.a [10] }"
    );

    auto query_container = parse_cpl(&ss);
    auto root_operator = query_container->get_root_operator_by_stream_name("result");
    RedoArea redo_area(root_operator.get());
    auto redo_streams = redo_area.get_redo_streams();

    EXPECT_EQ(1, redo_streams.size());

    EXPECT_EQ(query_container->get_stream_by_name("A"), redo_streams[0]);
}
