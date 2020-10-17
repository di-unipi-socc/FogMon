#ifndef ITHING_HPP_
#define ITHING_HPP_

#include <string>

/**
 * A simple interface for IoT devices
*/
class IThing{

public:

    /**
     * a method to get the latency of the device
     * @return the latency
    */
    virtual int getLatency() = 0;
    /**
     * a method to get the description of the device
     * @return the description
    */
    virtual std::string getDesc() = 0;
    /**
     * a method to get the id of the device
     * @return the id
    */
    virtual std::string getId() = 0;

    /**
     * a method to get fill the information on the device
    */
    virtual void monitor() = 0;

};


#endif