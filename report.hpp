#ifndef REPORT_HPP_
#define REPORT_HPP_

#include "rapidjson/document.h"

class Report {
public:
    Report();
    ~Report();

    bool parseJson(rapidjson::Value& data);
    rapidjson::Value getJson();
};

#endif