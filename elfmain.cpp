#include <iostream>
#include "inputParser.hpp"
#include "elf.hpp"

#include <stdlib.h>
#include <sys/time.h>

#include <unistd.h>
#include "message.hpp"

using namespace std;


int main(int argc, char *argv[]) {

    InputParser input(argc,argv);

    printf("hola");



    char json[512];

    sprintf(json, "{ \"type\": %d, \"command\": %d, \"argument\": %d, \"data\": {\"A\":[\"aa\",\"b\",\"c\"],\"B\":[\"a1\",\"b1\",\"c1\"]} }", Message::Type::REQUEST, Message::Command::SET, Message::Argument::NODES);

    Message m;
    bool res = m.ParseJson(json);
    cout << res << " " << m.getType() << " " << m.getCommand() << " " << m.getArgument() << " " <<endl;
    vector<string> vec;
    vector<string> vec2;
    res = m.getData(vec,vec2);
    cout << res<< endl;
    for(auto &v : vec) {
        cout << v << endl;
    }
    for(auto &v : vec2) {
        cout << v << endl;
    }
    




    Elf elf(2);
    elf.start();
    int a;
    scanf("%d",&a);
    elf.stop();
    printf("ciao");
    return 0;
}
