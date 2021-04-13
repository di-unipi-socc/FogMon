#ifndef MICROBIT_HPP_
#define MICROBIT_HPP_

#include <string>
#include "ithing.hpp"

class Microbit : public IThing {
private:

    std::string port;
    int latency;
    std::string desc;
    std::string id;

    int read();

public:

    Microbit(std::string port);

    int getLatency();
    std::string getDesc();
    std::string getId();

    //fill and update the information of this thing
    void monitor();

};


#endif