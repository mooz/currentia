#include <gtest/gtest.h>

#include <sstream>
#include "currentia/server/server.h"

using namespace currentia;

InputStreamHandler::ptr_t define_stream_from_ddl(Server& server, std::istream& stream_ptr) {
    CreateNode::ptr_t create_node = Parser::parse_create_from_stream(stream_ptr);

    return server.define_stream(create_node->name, create_node->attributes);
}

TEST (TestServer, DefineStream) {
    Server server;

    try {
        std::stringstream is1("CREATE STREAM TESTSTREAM1(ID INT, NAME STRING)");
        InputStreamHandler::ptr_t test_stream1 = define_stream_from_ddl(server, is1);
        EXPECT_TRUE(server.has_stream_with_name("TESTSTREAM1"));

        std::stringstream is2("CREATE STREAM TESTSTREAM2(ID INT, NAME STRING, AGE INT)");
        InputStreamHandler::ptr_t test_stream2 = define_stream_from_ddl(server, is2);
        EXPECT_TRUE(server.has_stream_with_name("TESTSTREAM2"));


    } catch (std::string error) {
        std::cout << error << std::endl;
        EXPECT_TRUE(false);
    }

    // EXPECT_EQ();
}
