#include "message.hpp"

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

Message::Message() {
    this->argument = Argument::NONE;
    this->data = Value("none");
    this->doc.SetObject();
}

Message::~Message() {

}

void Message::Clear() {

}

bool Message::parseJson(char *json) {
    ParseResult ok = doc.Parse((const char*)json);
    if(!ok)
        return false;
    
    if( !doc.HasMember("type") && !doc["type"].IsInt() &&
        !doc.HasMember("command") && !doc["command"].IsInt() &&
        !doc.HasMember("argument") && !doc["argument"].IsInt())
        return false;
    
    this->type = (Type)doc["type"].GetInt();
    this->command = (Command)doc["command"].GetInt();
    this->argument = (Argument)doc["argument"].GetInt();

    if(!doc.HasMember("data"))
        return false;

    this->data = doc["data"]; 

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

bool Message::getData(int& i) {
    if(!this->data.IsInt())
        return false;
    i = data.GetInt();
    return true;
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

bool Message::getData(string& stringA, vector<string>& stringsB) {
    if( !this->data.HasMember("A") && !this->data["A"].IsString() &&
        !this->data.HasMember("B") && !this->data["B"].IsArray())
        return false;

    stringA = this->data["A"].GetString();

    for (auto& v : this->data["B"].GetArray()) {
        if(!v.IsString())
            return false;
        stringsB.push_back(v.GetString());
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

 bool Message::getData(Report& report) {
     return report.parseJson(this->data);
 }

void Message::setData(int i) {

    Value val(i);
    Document::AllocatorType& allocator = doc.GetAllocator();                                                                                                                                                                                                                                                                                                                                                                                                                                        
    
    this->data = val;
}

void Message::setData(std::vector<std::string> strings) {

    Value arr(kArrayType);
    Document::AllocatorType& allocator = doc.GetAllocator();
    
    for(auto str : strings) {
        Value val(str.c_str(), allocator);
        arr.PushBack(val, allocator);
    }                                                                                                                                                                                                                                                                                                                                                                                                                                                        
    this->data = arr;
    
}

void Message::setData(string stringA, vector<string> stringsB) {

    Value obj(kObjectType);
    Value strA(kStringType);
    Value arrB(kArrayType);
    Document::AllocatorType& allocator = doc.GetAllocator();
    
    strA.SetString(stringA.c_str(), allocator);

    for(auto str : stringsB) {
        Value val(str.c_str(), allocator);
        arrB.PushBack(val, allocator);
    }

    obj.AddMember("A", strA, allocator);
    obj.AddMember("B", arrB, allocator);
    this->data = obj;
}

void Message::setData(std::vector<std::string> stringsA, std::vector<std::string> stringsB) {

    Value obj(kObjectType);
    Value arrA(kArrayType);
    Value arrB(kArrayType);
    Document::AllocatorType& allocator = doc.GetAllocator();
    
    for(auto str : stringsA) {
        Value val(str.c_str(), allocator);
        arrA.PushBack(val, allocator);
    }
    for(auto str : stringsB) {
        Value val(str.c_str(), allocator);
        arrB.PushBack(val, allocator);
    }

    obj.AddMember("A", arrA, allocator);
    obj.AddMember("B", arrB, allocator);
    this->data = obj;
}

void Message::setData(Report& report) {
    Value obj(kObjectType);
    obj.CopyFrom(*report.getJson(),doc.GetAllocator());
    this->data = obj;
}

void Message::buildString() {
    doc.RemoveAllMembers();
    
    doc.AddMember("type", this->type, doc.GetAllocator());
    doc.AddMember("command", this->command, doc.GetAllocator());
    doc.AddMember("argument", this->argument, doc.GetAllocator());

    doc.AddMember("data", this->data, doc.GetAllocator());
}

string Message::getString() {
    StringBuffer s;
    rapidjson::Writer<StringBuffer> writer (s);
    doc.Accept (writer);
    std::string str (s.GetString());
    return str;
}