#include "inputParser.hpp"
#include "master_node.hpp"

int main(int argc, char *argv[]) {

    MasterNode node(2);

    node.start();

    int a;
    scanf("%d",&a);

    node.stop();
    return 0;
}