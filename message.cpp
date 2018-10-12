#include "message.hpp"

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

Message::Message() {

}

Message::~Message() {

}

void Message::Clear() {

}

bool Message::ParseJson(char *json) {
    Document d;
    ParseResult ok = d.Parse((const char*)json);
    if(!ok)
        return false;
    
    if( !d.HasMember("type") && !d["type"].IsInt() &&
        !d.HasMember("command") && !d["command"].IsInt() &&
        !d.HasMember("argument") && !d["argument"].IsInt())
        return false;
    
    this->type = (Type)d["type"].GetInt();
    this->command = (Command)d["command"].GetInt();
    this->argument = (Argument)d["argument"].GetInt();


    if(!d.HasMember("data"))
        return false;
    
    this->data = d["data"]; 

    return true;
}

void Message::setType(Type t) {
    type = t;
}

Message::Type Message::getType() {
    return type;
}

void Message::setCommand(Command c) {
    command = c;
}

Message::Command Message::getCommand() {
    return command;
}

void Message::setArgument(Argument a) {
    argument = a;
}

Message::Argument Message::getArgument() {
    return argument;
}

bool Message::getData(vector<string>& strings) {
    if(!this->data.IsArray())
        return false;
    for (auto& v : this->data.GetArray()) {
        if(!v.IsString())
            return false;
        strings.push_back(v.GetString());
    }
    return true;
}

bool Message::getData(vector<string>& stringsA, vector<string>& stringsB) {
    if( !this->data.HasMember("A") && !this->data["A"].IsArray() &&
        !this->data.HasMember("B") && !this->data["B"].IsArray())
        return false;

    for (auto& v : this->data["A"].GetArray()) {
        if(!v.IsString())
            return false;
        stringsA.push_back(v.GetString());
    }

    for (auto& v : this->data["B"].GetArray()) {
        if(!v.IsString())
            return false;
        stringsB.push_back(v.GetString());
    }
    return true;
}