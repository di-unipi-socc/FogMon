#ifndef IOTDISCOVERER_HPP_
#define IOTDISCOVERER_HPP_

#include <vector>
#include "iiotdiscoverer.hpp"

class MicrobitDiscoverer : public IIoTDiscoverer{

public:
    std::vector<IThing*> discover();

    MicrobitDiscoverer();

};


#endif