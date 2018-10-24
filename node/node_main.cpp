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

    string ip;

    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    Node node(ip,2);
    node.start();

    int a;
    scanf("%d",&a);

    node.stop();

    return 0;
}
