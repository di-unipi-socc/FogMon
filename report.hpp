#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "rapidjson/document.h"

class Report {
    Report();
    ~Report();

    bool jsonParse(rapidjson::Value data);
    
};

#endif