#ifndef ITHING_HPP_
#define ITHING_HPP_

#include <string>

class IThing{

public:

    virtual int getLatency() = 0;
    virtual std::string getDesc() = 0;
    virtual std::string getId() = 0;

    //fill and update the information of this thing
    virtual void monitor() = 0;

};


#endif