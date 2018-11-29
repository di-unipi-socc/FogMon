#include <gtest/gtest.h>
#include "connections.hpp"
#include "master_connections.hpp"

TEST(ConnectionsTest, Request1) {
    int pipefd[2];

    EXPECT_EQ(pipe (pipefd), 0);
}