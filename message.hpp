#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "report.hpp"

class Message {
public:
    enum Type {REQUEST, NOTIFY};
    enum Command {GET, SET, HELLO, NODELIST, UPDATE, RESPONSE};
    enum Argument {NONE, NODES, REPORT, POSITIVE, NEGATIVE};

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

    bool getData(std::vector<std::string>& strings);
    bool getData(std::vector<std::string>& stringsA, std::vector<std::string>& stringsB);
    bool getData(Report& report);

    void setData(std::vector<std::string> strings);
    void setData(std::vector<std::string> stringsA, std::vector<std::string> stringsB);
    void setData(Report report);

private:

    Type type;
    Command command;
    Argument argument;

    rapidjson::Document doc;
    rapidjson::Value data;

};

#endif