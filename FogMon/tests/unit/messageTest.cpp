#include <gtest/gtest.h>
#include "message.hpp"
#include <vector>
#include <string>

using namespace std;

TEST(MessageTest, ParseGetInt) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"sender\":{\"id\":\"sender\",\"ip\":\"1\",\"port\":\"2\"}, \"type\":1, \"command\":2, \"argument\":3, \"data\":2}"), true);

    EXPECT_EQ(mes.getSender().id, "sender");
    EXPECT_EQ(mes.getSender().ip, "1");
    EXPECT_EQ(mes.getSender().port, "2");
    EXPECT_EQ(mes.getType(), 1);
    EXPECT_EQ(mes.getCommand(), 2);
    EXPECT_EQ(mes.getArgument(), 3);
    int i;
    EXPECT_EQ(mes.getData(i), true);
    EXPECT_EQ(i, 2);
}

TEST(MessageTest, ParseGetVector) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"sender\":{\"id\":\"sender\",\"ip\":\"\",\"port\":\"\"}, \"type\":1, \"command\":2, \"argument\":3, \"data\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}]}"), true);
    
    vector<Message::node> strings;

    EXPECT_EQ(mes.getData(strings), true);

    for(int i=0; i<strings.size(); i++) {
        EXPECT_EQ(strings[i].id, to_string(i));
    }
}


TEST(MessageTest, ParseGetVectorAndString) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"sender\":{\"id\":\"sender\",\"ip\":\" \",\"port\":\" \"}, \"type\":1, \"command\":2, \"argument\":3, \"data\":{\"A\":{\"id\":\"hello\",\"ip\":\"\",\"port\":\"\"},\"B\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}]}}"), true);
    
    Message::node stringA;
    vector<Message::node> stringsB;
    EXPECT_EQ(mes.getData(stringA,stringsB), true);

    EXPECT_EQ(stringA.id, "hello");

    for(int i=0; i<stringsB.size(); i++) {
        EXPECT_EQ(stringsB[i].id, to_string(i));
    }
}

TEST(MessageTest, ParseGetVectors) {
    Message mes;
    EXPECT_EQ(mes.parseJson((char*)"{\"sender\":{\"id\":\"sender\",\"ip\":\"\",\"port\":\"\"}, \"type\":1, \"command\":2, \"argument\":3, \"data\":{\"A\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}],\"B\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}]}}"), true);
    
    vector<Message::node> stringsA;
    vector<Message::node> stringsB;
    EXPECT_EQ(mes.getData(stringsA,stringsB), true);

    for(int i=0; i<stringsA.size(); i++) {
        EXPECT_EQ(stringsA[i].id, to_string(i));
    }
    for(int i=0; i<stringsB.size(); i++) {
        EXPECT_EQ(stringsB[i].id, to_string(i));
    }
}

TEST(MessageTest, ParseToString) {
    Message mes;
    char json[] = "{\"sender\":{\"id\":\"sender\",\"ip\":\"\",\"port\":\"\"},\"type\":1,\"command\":2,\"argument\":3,\"data\":{\"A\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}],\"B\":[{\"id\":\"0\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"1\",\"ip\":\"\",\"port\":\"\"},{\"id\":\"2\",\"ip\":\"\",\"port\":\"\"}]}}";
    EXPECT_EQ(mes.parseJson((char*)json), true);

    mes.buildString();
    EXPECT_EQ(mes.getString(), json);
}

TEST(MessageTest, SetGetInt) {
    Message mes;
    mes.setSender(Message::node("sender","1","2"));
    mes.setType((Message::Type)1);
    mes.setCommand((Message::Command)2);
    mes.setArgument((Message::Argument)3);
    mes.setData(2);

    EXPECT_EQ(mes.getSender().id, "sender");
    EXPECT_EQ(mes.getSender().ip, "1");
    EXPECT_EQ(mes.getSender().port, "2");
    EXPECT_EQ(mes.getType(), 1);
    EXPECT_EQ(mes.getCommand(), 2);
    EXPECT_EQ(mes.getArgument(), 3);
    int i;
    EXPECT_EQ(mes.getData(i), true);
    EXPECT_EQ(i, 2);
}

TEST(MessageTest, SetGetVector) {
    Message mes;
    
    vector<Message::node> strings2;
    for(int i=0; i<4; i++) {
        strings2.push_back(Message::node(to_string(i),"",""));
    }
    mes.setData(strings2);    

    vector<Message::node> strings;

    EXPECT_EQ(mes.getData(strings), true);

    for(int i=0; i<strings.size(); i++) {
        EXPECT_EQ(strings[i], strings2[i]);
    }
}


TEST(MessageTest, SetGetVectorAndString) {
    Message mes;
    
    vector<Message::node> strings2;
    for(int i=0; i<4; i++) {
        strings2.push_back(Message::node(to_string(i),"",""));
    }
    mes.setData(Message::node("hello","",""), strings2);   
    
    Message::node stringA;
    vector<Message::node> stringsB;
    EXPECT_EQ(mes.getData(stringA,stringsB), true);

    EXPECT_EQ(stringA.id, "hello");

    for(int i=0; i<strings2.size(); i++) {
        EXPECT_EQ(stringsB[i], strings2[i]);
    }
}

TEST(MessageTest, SetGetVectors) {
    Message mes;
    vector<Message::node> strings2A,strings2B;
    for(int i=0; i<4; i++) {
        strings2A.push_back(Message::node(to_string(i),"",""));
    }
    for(int i=0; i<4; i++) {
        strings2B.push_back(Message::node(to_string(i),"",""));
    }

    mes.setData(strings2A, strings2B);   
    
    vector<Message::node> stringsA;
    vector<Message::node> stringsB;
    EXPECT_EQ(mes.getData(stringsA,stringsB), true);

    for(int i=0; i<strings2A.size(); i++) {
        EXPECT_EQ(stringsA[i], strings2A[i]);
    }
    for(int i=0; i<strings2B.size(); i++) {
        EXPECT_EQ(stringsB[i], strings2B[i]);
    }
}