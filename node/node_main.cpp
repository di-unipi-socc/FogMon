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

    printf("hola");



    char json[512];

    sprintf(json, "{ \"type\": %d, \"command\": %d, \"argument\": %d, \"data\": {\"A\":[\"aa\",\"b\",\"c\"],\"B\":[\"a1\",\"b1\",\"c1\"]} }", Message::Type::REQUEST, Message::Command::SET, Message::Argument::NODES);

    Document d;
    ParseResult ok = d.Parse((const char*)json);

    Document d2;
    ok = d2.Parse((const char*)json);

    d.RemoveMember("data");
    d.AddMember("data",d2, d.GetAllocator());
    d.AddMember("data2",42, d.GetAllocator());

    StringBuffer s;
    rapidjson::Writer<StringBuffer> writer (s);
    d.Accept (writer);
    std::string json2 (s.GetString());
    cout << json2 << endl << "fin" << endl;



    Message m;
    bool res = m.parseJson(json);
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



    Node node(2);
    node.start();
    int a;
    scanf("%d",&a);
    node.stop();
    printf("ciao");
    return 0;
}
