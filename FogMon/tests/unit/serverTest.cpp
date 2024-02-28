#include <gtest/gtest.h>
#include "server.hpp"

#include "connections.hpp"
#include "follower_connections.hpp"
#include "follower.hpp"
#include <iostream>

using namespace std;

class MyConn : public Connections {
public:

    MyConn() : Connections(1) {

    }

    void handler(int fd, Message &m) {
        string strIp = this->getSource(fd,m);

        cout << strIp << endl;
        EXPECT_EQ(strIp, "::1");

        Message res;

        if( m.getType()==Message::Type::REQUEST &&
            m.getCommand() == Message::Command::GET &&
            m.getArgument() == Message::Argument::NODES) {
            
            
        }else {
            FAIL();
        }
        
    }
};

TEST(ServerTest, serverTest) {
    IConnections * myConn = new MyConn();

    
    Server s(myConn, 12345);

    s.start();

    FollowerConnections fcon(0);
    IAgent * myFol= new Follower(Message::node("aa","bb","cc"), 0);
    fcon.initialize(myFol);
    vector<Message::node> vec = fcon.requestNodes(Message::node("a","::1","12345"));
    EXPECT_EQ(vec.size(), 0);
    s.stop();

    delete myFol;
    delete myConn;
}