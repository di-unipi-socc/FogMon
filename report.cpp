#include "report.hpp"

using namespace rapidjson;
using namespace std;

Report::Report() {
    doc.SetObject();
}

Report::~Report() {

}

bool Report::parseJson(Value& data) {
    doc.CopyFrom(data,doc.GetAllocator());
    if(doc.IsObject())
        return true;
    return false;
}

Value* Report::getJson() {
    return &doc;
}

void Report::setHardware(hardware_result hardware) {

    Value obj(kObjectType);
    doc.RemoveMember("hardware");
    
    Value cores(hardware.cores);
    Value free_cpu(hardware.free_cpu);
    Value memory(hardware.memory);
    Value free_memory(hardware.free_memory);
    Value disk(hardware.disk);
    Value free_disk(hardware.free_disk);

    obj.AddMember("cores", cores, doc.GetAllocator());
    obj.AddMember("free_cpu", free_cpu, doc.GetAllocator());
    obj.AddMember("memory", memory, doc.GetAllocator());
    obj.AddMember("free_memory", free_memory, doc.GetAllocator());
    obj.AddMember("disk", disk, doc.GetAllocator());
    obj.AddMember("free_disk", free_disk, doc.GetAllocator());

    doc.AddMember("hardware", obj, doc.GetAllocator());
}

void Report::setLatency(vector<test_result> latency) {

    Value arr(kArrayType);
    doc.RemoveMember("latency");

    Document::AllocatorType& allocator = doc.GetAllocator();

    for(auto test : latency) {
        Value target(test.target.c_str(), allocator);
        Value mean(test.mean);
        Value variance(test.variance);
        Value lasttime(test.lasttime);
        Value obj(kObjectType);
        obj.AddMember("target",target, allocator);
        obj.AddMember("mean",mean, allocator);
        obj.AddMember("variance",variance, allocator);
        obj.AddMember("lasttime",lasttime, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("latency", arr, allocator);
}

void Report::setBandwidth(vector<test_result> bandwidth) {

    Value arr(kArrayType);
    doc.RemoveMember("bandwidth");

    Document::AllocatorType& allocator = doc.GetAllocator();

    for(auto test : bandwidth) {
        Value target(test.target.c_str(), allocator);
        Value mean(test.mean);
        Value variance(test.variance);
        Value lasttime(test.lasttime);
        Value obj(kObjectType);
        obj.AddMember("target",target, allocator);
        obj.AddMember("mean",mean, allocator);
        obj.AddMember("variance",variance, allocator);
        obj.AddMember("lasttime",lasttime, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("bandwidth", arr, doc.GetAllocator());

}

bool Report::getHardware(hardware_result& hardware) {
    if( !this->doc.HasMember("hardware") || !this->doc["hardware"].IsObject())
        return false;

    Value &val = doc["hardware"];
    
    if( !val.HasMember("cores") || !val["cores"].IsInt() ||
        !val.HasMember("free_cpu") || !val["free_cpu"].IsFloat() ||
        !val.HasMember("memory") || !val["memory"].IsInt() ||
        !val.HasMember("free_memory") || !val["free_memory"].IsInt() ||
        !val.HasMember("disk") || !val["disk"].IsInt() ||
        !val.HasMember("free_disk") || !val["free_disk"].IsInt())
        return false;

    hardware.cores = val["cores"].GetInt();
    hardware.free_cpu = val["free_cpu"].GetFloat();
    hardware.memory = val["memory"].GetInt();
    hardware.free_memory = val["free_memory"].GetInt();
    hardware.disk = val["disk"].GetInt();
    hardware.free_disk = val["free_disk"].GetInt();
    
    return true;
}

bool Report::getLatency(vector<test_result>& latency) {
    if( !this->doc.HasMember("latency") || !this->doc["latency"].IsArray())
        return false;

    for (auto& v : this->doc["latency"].GetArray()) {
        int a = !v.HasMember("target");
        int b = !v["target"].IsString();
        int c = !v.HasMember("mean") || !v["mean"].IsFloat() ||
            !v.HasMember("variance") || !v["variance"].IsFloat();
        int d = !v.IsObject();
        if( !v.IsObject() ||
            !v.HasMember("target") || !v["target"].IsString() ||
            !v.HasMember("mean") || !v["mean"].IsFloat() ||
            !v.HasMember("variance") || !v["variance"].IsFloat() ||
            !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
            return false;
        test_result test;
        test.target = string(v["target"].GetString());
        test.mean = v["mean"].GetFloat();
        test.variance = v["variance"].GetFloat();
        test.lasttime = v["lasttime"].GetInt64();

        latency.push_back(test);
    }

    return true;
}

bool Report::getBandwidth(vector<test_result>& bandwidth) {
    if( !this->doc.HasMember("bandwidth") || !this->doc["bandwidth"].IsArray())
        return false;

    for (auto& v : this->doc["bandwidth"].GetArray()) {
        if( !v.IsObject() ||
            !v.HasMember("target") || !v["target"].IsString() ||
            !v.HasMember("mean") || !v["mean"].IsFloat() ||
            !v.HasMember("variance") || !v["variance"].IsFloat() ||
            !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
            return false;
        test_result test;
        test.target = string(v["target"].GetString());
        test.mean = v["mean"].GetFloat();
        test.variance = v["variance"].GetFloat();
        test.lasttime = v["lasttime"].GetInt64();

        bandwidth.push_back(test);
    }
    return true;
}