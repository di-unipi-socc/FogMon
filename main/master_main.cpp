#include "inputParser.hpp"
#include "master_node.hpp"

#include <unistd.h>


int main(int argc, char *argv[]) {

    MasterNode node(1);
    node.initialize();
    node.start();

    int a;
    scanf("%d",&a);
    scanf("%d",&a);

    node.stop();
    return 0;
}