#ifndef IIOTDISCOVERER_HPP_
#define IIOTDISCOVERER_HPP_

#include <vector>
#include "ithing.hpp"

class IIoTDiscoverer{

public:
    virtual std::vector<IThing*> discover() = 0;

};


#endif