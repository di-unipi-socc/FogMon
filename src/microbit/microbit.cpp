#include "microbit.hpp"

#include <libserialport.h>

#include <regex>

using namespace std;

Microbit::Microbit(string port) {
    this->port = port;
    //initialize the other values
    this->monitor();
}

int Microbit::getLatency() {
    return this->latency;
}

string Microbit::getDesc() {
    return this->desc;
}

string Microbit::getId() {
    return this->id;
}

void Microbit::monitor() {
    struct sp_port *port;

    int retval = sp_get_port_by_name(this->port.c_str(),&port);
    this->latency = -1;

    if(retval == SP_OK) {
        char * desc = sp_get_port_description(port);
        if(desc != NULL) {
            this->desc = string(desc);
            //get id
             std::regex reg("- ([0-9a-zA-Z]*)");

            std::smatch m;
            if (std::regex_search (this->desc,m,reg)) {
                this->id = m[1];

                //read and calc latency

                this->latency = 1; //TODO
            }
        }
        
        sp_free_port(port);
    }

    if(this->latency == -1) {
        this->desc = "";
        this->id = "";
    }
}