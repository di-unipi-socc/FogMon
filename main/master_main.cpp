#include "inputParser.hpp"
#include "master_node.hpp"

#include <unistd.h>


int main(int argc, char *argv[]) {
    InputParser input(argc,argv);

    string ip = "::1";
    int threads = 2;
    if(input.cmdOptionExists("-C"))
        ip = input.getCmdOption("-C");

    if(input.cmdOptionExists("-t"))
        threads = stoi(input.getCmdOption("-t"));


    MasterNode node(threads);
    node.initialize();
    node.start(ip);

    int a;
    scanf("%d",&a);
    scanf("%d",&a);

    node.stop();
    return 0;
}