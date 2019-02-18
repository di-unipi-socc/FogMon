#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "report.hpp"

class Message {
public:
    enum Type {REQUEST, NOTIFY, RESPONSE, MREQUEST, MNOTIFY, MRESPONSE};
    enum Command {GET, SET, HELLO, NODELIST, MNODELIST, UPDATE, START};
    enum Argument {NONE, NODES, MNODES, REPORT, POSITIVE, NEGATIVE, TOKEN, IPERF, ESTIMATE, LATENCY, BANDWIDTH};

    Message();
    ~Message();

    void Clear();
    bool parseJson(char* json);

    void buildString();
    std::string getString();

    void setType(Type t);
    void setCommand(Command c);
    void setArgument(Argument a);

    Type getType();
    Command getCommand();
    Argument getArgument();

    bool getData(int& i);
    bool getData(std::vector<std::string>& strings);
    bool getData(std::string& stringA, std::vector<std::string>& stringsB);
    bool getData(std::vector<std::string>& stringsA, std::vector<std::string>& stringsB);
    bool getData(Report& report);

    void setData(int i);
    void setData(std::vector<std::string> strings);
    void setData(std::string string, std::vector<std::string> strings);
    void setData(std::vector<std::string> stringsA, std::vector<std::string> stringsB);
    void setData(Report& report);

private:

    Type type;
    Command command;
    Argument argument;

    rapidjson::Document doc;
    rapidjson::Value data;

};

#endif