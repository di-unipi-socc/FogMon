#include "report.hpp"

using namespace rapidjson;

Report::Report() {

}

Report::~Report() {

}

bool Report::parseJson(Value& data) {

    return true;
}

Value Report::getJson() {
    Value val(10);
    return val;
}