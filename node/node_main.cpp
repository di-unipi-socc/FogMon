#include <iostream>
#include "inputParser.hpp"
#include "node.hpp"

#include <stdlib.h>
#include <sys/time.h>

#include <unistd.h>
#include "message.hpp"

using namespace std;

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <iostream>

using namespace rapidjson;

#include <sstream>
#include <arpa/inet.h>

bool is_ipv4_address(const string& str)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr))!=0;
}

bool is_ipv6_address(const string& str)
{
    struct sockaddr_in6 sa;
    return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr))!=0;
}

int main(int argc, char *argv[]) {

    cout << is_ipv4_address("192.168.124.1") << is_ipv6_address("192.168.124.1") << endl;

    InputParser input(argc,argv);

    string ip = "localhost:5556";

    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    Node node(ip,2);
    node.start();

    int a;
    scanf("%d",&a);

    node.stop();

    return 0;
}
