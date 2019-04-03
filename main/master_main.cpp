#include "inputParser.hpp"
#include "master_node.hpp"

#include <unistd.h>


int main(int argc, char *argv[]) {

    MasterNode node(1);
    node.initialize();
    node.start();

    while(true) {
        sleep(10);
    }

    node.stop();
    return 0;
}