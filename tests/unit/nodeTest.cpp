#include <gtest/gtest.h>

#include "node.hpp"
#include "master_node.hpp"

TEST(NodeMasterTest, Aaaa) {
    MasterNode Node2(1);
    Node2.initialize();

    EXPECT_EQ(1,0);
}