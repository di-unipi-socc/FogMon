#include <gtest/gtest.h>
#include "message.hpp"
#include <vector>
#include <string>

using namespace std;

TEST(MessageTest, ParseGetInt) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"type\":1, \"command\":2, \"argument\":3, \"data\":2}"), true);

    EXPECT_EQ(mes.getType(), 1);
    EXPECT_EQ(mes.getCommand(), 2);
    EXPECT_EQ(mes.getArgument(), 3);
    int i;
    EXPECT_EQ(mes.getData(i), true);
    EXPECT_EQ(i, 2);
}

TEST(MessageTest, ParseGetVector) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"type\":1, \"command\":2, \"argument\":3, \"data\":[\"0\",\"1\",\"2\",\"3\"]}"), true);
    
    vector<string> strings;

    EXPECT_EQ(mes.getData(strings), true);

    for(int i=0; i<strings.size(); i++) {
        EXPECT_EQ(strings[i], to_string(i));
    }
}


TEST(MessageTest, ParseGetVectorAndString) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"type\":1, \"command\":2, \"argument\":3, \"data\":{\"A\":\"hello\",\"B\":[\"0\",\"1\",\"2\",\"3\"]}}"), true);
    
    string stringA;
    vector<string> stringsB;
    EXPECT_EQ(mes.getData(stringA,stringsB), true);

    EXPECT_EQ(stringA, "hello");

    for(int i=0; i<stringsB.size(); i++) {
        EXPECT_EQ(stringsB[i], to_string(i));
    }
}

TEST(MessageTest, ParseGetVectors) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"type\":1, \"command\":2, \"argument\":3, \"data\":{\"A\":[\"0\",\"1\",\"2\",\"3\"],\"B\":[\"0\",\"1\",\"2\",\"3\"]}}"), true);
    
    vector<string> stringsA;
    vector<string> stringsB;
    EXPECT_EQ(mes.getData(stringsA,stringsB), true);

    for(int i=0; i<stringsA.size(); i++) {
        EXPECT_EQ(stringsA[i], to_string(i));
    }
    for(int i=0; i<stringsB.size(); i++) {
        EXPECT_EQ(stringsB[i], to_string(i));
    }
}

TEST(MessageTest, ParseToString) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"type\":1, \"command\":2, \"argument\":3, \"data\":{\"A\":[\"0\",\"1\",\"2\",\"3\"],\"B\":[\"0\",\"1\",\"2\",\"3\"]}}"), true);

    mes.buildString();
    EXPECT_EQ(mes.getString(), "{\"type\":1,\"command\":2,\"argument\":3,\"data\":{\"A\":[\"0\",\"1\",\"2\",\"3\"],\"B\":[\"0\",\"1\",\"2\",\"3\"]}}");
}

TEST(MessageTest, SetGetInt) {
    Message mes;
    mes.setType((Message::Type)1);
    mes.setCommand((Message::Command)2);
    mes.setArgument((Message::Argument)3);
    mes.setData(2);

    EXPECT_EQ(mes.getType(), 1);
    EXPECT_EQ(mes.getCommand(), 2);
    EXPECT_EQ(mes.getArgument(), 3);
    int i;
    EXPECT_EQ(mes.getData(i), true);
    EXPECT_EQ(i, 2);
}

TEST(MessageTest, SetGetVector) {
    Message mes;
    
    vector<string> strings2;
    for(int i=0; i<4; i++) {
        strings2.push_back(to_string(i));
    }
    mes.setData(strings2);    

    vector<string> strings;

    EXPECT_EQ(mes.getData(strings), true);

    for(int i=0; i<strings.size(); i++) {
        EXPECT_EQ(strings[i], strings2[i]);
    }
}


TEST(MessageTest, SetGetVectorAndString) {
    Message mes;
    
    vector<string> strings2;
    for(int i=0; i<4; i++) {
        strings2.push_back(to_string(i));
    }
    mes.setData("hello", strings2);   
    
    string stringA;
    vector<string> stringsB;
    EXPECT_EQ(mes.getData(stringA,stringsB), true);

    EXPECT_EQ(stringA, "hello");

    for(int i=0; i<strings2.size(); i++) {
        EXPECT_EQ(stringsB[i], strings2[i]);
    }
}

TEST(MessageTest, SetGetVectors) {
    Message mes;
    vector<string> strings2A,strings2B;
    for(int i=0; i<4; i++) {
        strings2A.push_back(to_string(i));
    }
    for(int i=0; i<4; i++) {
        strings2B.push_back(to_string(i));
    }

    mes.setData(strings2A, strings2B);   
    
    vector<string> stringsA;
    vector<string> stringsB;
    EXPECT_EQ(mes.getData(stringsA,stringsB), true);

    for(int i=0; i<strings2A.size(); i++) {
        EXPECT_EQ(stringsA[i], strings2A[i]);
    }
    for(int i=0; i<strings2B.size(); i++) {
        EXPECT_EQ(stringsB[i], strings2B[i]);
    }
}