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

int main(int argc, char *argv[]) {
    InputParser input(argc,argv);

    string ip = "localhost";
    int threads = 2;
    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    if(input.cmdOptionExists("-t"))
        threads = stoi(input.getCmdOption("-t"));

    Node node(ip, threads);
    node.initialize();
    node.start();

    int a;
    scanf("%d",&a);

    node.stop();

    return 0;
}
