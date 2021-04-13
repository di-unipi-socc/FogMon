#include "report.hpp"

#include "rapidjson/reader.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

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
    Value mean_free_cpu(hardware.mean_free_cpu);
    Value var_free_cpu(hardware.var_free_cpu);
    Value memory(hardware.memory);
    Value mean_free_memory(hardware.mean_free_memory);
    Value var_free_memory(hardware.var_free_memory);
    Value disk(hardware.disk);
    Value mean_free_disk(hardware.mean_free_disk);
    Value var_free_disk(hardware.var_free_disk);

    Document::AllocatorType& allocator = doc.GetAllocator();

    obj.AddMember("cores", cores, allocator);
    obj.AddMember("mean_free_cpu", mean_free_cpu, allocator);
    obj.AddMember("var_free_cpu", var_free_cpu, allocator);
    obj.AddMember("memory", memory, allocator);
    obj.AddMember("mean_free_memory", mean_free_memory, allocator);
    obj.AddMember("var_free_memory", var_free_memory, allocator);
    obj.AddMember("disk", disk, allocator);
    obj.AddMember("mean_free_disk", mean_free_disk, allocator);
    obj.AddMember("var_free_disk", var_free_disk, allocator);
    obj.AddMember("lasttime", hardware.lasttime, allocator);

    doc.AddMember("hardware", obj, allocator);
}

void Report::setLatency(vector<test_result> latency) {

    Value arr(kArrayType);
    doc.RemoveMember("latency");

    Document::AllocatorType& allocator = doc.GetAllocator();

    for(auto test : latency) {
        Value mean(test.mean);
        Value variance(test.variance);
        Value lasttime(test.lasttime);
        Value obj(kObjectType);
        obj.AddMember("target",test.target.getJson(allocator), allocator);
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
        Value mean(test.mean);
        Value variance(test.variance);
        Value lasttime(test.lasttime);
        Value obj(kObjectType);
        obj.AddMember("target",test.target.getJson(allocator), allocator);
        obj.AddMember("mean",mean, allocator);
        obj.AddMember("variance",variance, allocator);
        obj.AddMember("lasttime",lasttime, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("bandwidth", arr, doc.GetAllocator());

}

void Report::setIot(vector<IoT> iots) {

    Value arr(kArrayType);
    doc.RemoveMember("iot");

    Document::AllocatorType& allocator = doc.GetAllocator();

    for(auto iot : iots) {
        Value id(iot.id.c_str(), allocator);
        Value desc(iot.desc.c_str(), allocator);
        Value latency(iot.latency);
        Value obj(kObjectType);
        obj.AddMember("id",id, allocator);
        obj.AddMember("desc",desc, allocator);
        obj.AddMember("latency",latency, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("iot", arr, doc.GetAllocator());

}

void Report::setReport(report_result report) {
    this->setHardware(report.hardware);
    this->setLatency(report.latency);
    this->setBandwidth(report.bandwidth);
    this->setIot(report.iot);
}

void Report::setReports(std::vector<report_result> reports) {
    Value arr(kArrayType);
    doc.RemoveMember("reports");

    Document::AllocatorType& allocator = doc.GetAllocator();

    for(auto test : reports) {
        
        Value hw(kObjectType);
        Value lt(kArrayType);
        Value bw(kArrayType);
        Value th(kArrayType);

        {
            Report::hardware_result &hardware = test.hardware;
            
        
            Value cores(hardware.cores);
            Value mean_free_cpu(hardware.mean_free_cpu);
            Value var_free_cpu(hardware.var_free_cpu);
            Value memory(hardware.memory);
            Value mean_free_memory(hardware.mean_free_memory);
            Value var_free_memory(hardware.var_free_memory);
            Value disk(hardware.disk);
            Value mean_free_disk(hardware.mean_free_disk);
            Value var_free_disk(hardware.var_free_disk);

            hw.AddMember("cores", cores, allocator);
            hw.AddMember("mean_free_cpu", mean_free_cpu, allocator);
            hw.AddMember("var_free_cpu", var_free_cpu, allocator);
            hw.AddMember("memory", memory, allocator);
            hw.AddMember("mean_free_memory", mean_free_memory, allocator);
            hw.AddMember("var_free_memory", var_free_memory, allocator);
            hw.AddMember("disk", disk, allocator);
            hw.AddMember("mean_free_disk", mean_free_disk, allocator);
            hw.AddMember("var_free_disk", var_free_disk, allocator);
            hw.AddMember("lasttime", hardware.lasttime, allocator);
        }
        
        for(auto testLt : test.latency) {
            Value mean(testLt.mean);
            Value variance(testLt.variance);
            Value lasttime(testLt.lasttime);
            Value obj(kObjectType);
            obj.AddMember("target",testLt.target.getJson(allocator), allocator);
            obj.AddMember("mean",mean, allocator);
            obj.AddMember("variance",variance, allocator);
            obj.AddMember("lasttime",lasttime, allocator);

            lt.PushBack(obj, allocator);
        }

        for(auto testBw : test.bandwidth) {
            Value mean(testBw.mean);
            Value variance(testBw.variance);
            Value lasttime(testBw.lasttime);
            Value obj(kObjectType);
            obj.AddMember("target",testBw.target.getJson(allocator), allocator);
            obj.AddMember("mean",mean, allocator);
            obj.AddMember("variance",variance, allocator);
            obj.AddMember("lasttime",lasttime, allocator);

            bw.PushBack(obj, allocator);
        }

        for(auto iot : test.iot) {
            Value id(iot.id.c_str(), allocator);
            Value desc(iot.desc.c_str(), allocator);
            Value latency(iot.latency);
            Value obj(kObjectType);
            obj.AddMember("id",id, allocator);
            obj.AddMember("desc",desc, allocator);
            obj.AddMember("latency",latency, allocator);

            th.PushBack(obj, allocator);
        }

        
        Value obj(kObjectType);
        Value leader(test.leader.c_str(), allocator);

        obj.AddMember("source",test.source.getJson(allocator), allocator);
        obj.AddMember("hardware",hw, allocator);
        obj.AddMember("latency",lt, allocator);
        obj.AddMember("bandwidth",bw, allocator);
        obj.AddMember("iot",th, allocator);
        obj.AddMember("leader",leader, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("reports", arr, doc.GetAllocator());
}

bool Report::getHardware(hardware_result& hardware) {
    if( !this->doc.HasMember("hardware") || !this->doc["hardware"].IsObject())
        return false;

    Value &val = doc["hardware"];
    
    if( !val.HasMember("cores") || !val["cores"].IsInt() ||
        !val.HasMember("mean_free_cpu") || !val["mean_free_cpu"].IsFloat() ||
        !val.HasMember("var_free_cpu") || !val["var_free_cpu"].IsFloat() ||
        !val.HasMember("memory") || !val["memory"].IsInt64() ||
        !val.HasMember("mean_free_memory") || !val["mean_free_memory"].IsFloat() ||
        !val.HasMember("var_free_memory") || !val["var_free_memory"].IsFloat() ||
        !val.HasMember("disk") || !val["disk"].IsInt64() ||
        !val.HasMember("mean_free_disk") || !val["mean_free_disk"].IsFloat() ||
        !val.HasMember("var_free_disk") || !val["var_free_disk"].IsFloat() ||
        !val.HasMember("lasttime") || !val["lasttime"].IsInt64())
        return false;

    hardware.cores = val["cores"].GetInt();
    hardware.mean_free_cpu = val["mean_free_cpu"].GetFloat();
    hardware.var_free_cpu = val["var_free_cpu"].GetFloat();
    hardware.memory = val["memory"].GetInt64();
    hardware.mean_free_memory = val["mean_free_memory"].GetFloat();
    hardware.var_free_memory = val["var_free_memory"].GetFloat();
    hardware.disk = val["disk"].GetInt64();
    hardware.mean_free_disk = val["mean_free_disk"].GetFloat();
    hardware.var_free_disk = val["var_free_disk"].GetFloat();
    hardware.lasttime = val["lasttime"].GetInt64();
    
    return true;
}

bool Report::getLatency(vector<test_result>& latency) {
    if( !this->doc.HasMember("latency") || !this->doc["latency"].IsArray())
        return false;

    for (auto& v : this->doc["latency"].GetArray()) {
        if( !v.IsObject() ||
            !v.HasMember("target") || !v["target"].IsObject() ||
            !v.HasMember("mean") || !v["mean"].IsFloat() ||
            !v.HasMember("variance") || !v["variance"].IsFloat() ||
            !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
            return false;
        test_result test;
        test.target.setJson(v["target"]);
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
            !v.HasMember("target") || !v["target"].IsObject() ||
            !v.HasMember("mean") || !v["mean"].IsFloat() ||
            !v.HasMember("variance") || !v["variance"].IsFloat() ||
            !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
            return false;
        test_result test;
        test.target.setJson(v["target"]);
        test.mean = v["mean"].GetFloat();
        test.variance = v["variance"].GetFloat();
        test.lasttime = v["lasttime"].GetInt64();

        bandwidth.push_back(test);
    }
    return true;
}

bool Report::getIot(vector<IoT>& iots) {
    if( !this->doc.HasMember("iot") || !this->doc["iot"].IsArray())
        return false;
    
    for(auto& v: this->doc["iot"].GetArray()) {
        if( !v.IsObject() ||
            !v.HasMember("id") || !v["id"].IsString() ||
            !v.HasMember("desc") || !v["desc"].IsString() ||
            !v.HasMember("latency") || !v["latency"].IsInt())
            return false;
        IoT iot;
        iot.id = string(v["id"].GetString());
        iot.desc = string(v["desc"].GetString());
        iot.latency = v["latency"].GetInt();

        iots.push_back(iot);
    }
    return true;
}

bool Report::getReport(report_result &report) {
    
    if( !this->getHardware(report.hardware) ||
        !this->getLatency(report.latency) ||
        !this->getBandwidth(report.bandwidth)||
        !this->getIot(report.iot))
        return false;

    return true;
}
    
bool Report::getReports(std::vector<report_result> &reports) {
    if( !this->doc.HasMember("reports") || !this->doc["reports"].IsArray())
        return false;
    
    reports.clear();

    for (auto& v : this->doc["reports"].GetArray()) {
        if( !v.IsObject() ||
            !v.HasMember("hardware") || !v["hardware"].IsObject() ||
            !v.HasMember("latency") || !v["latency"].IsArray() ||
            !v.HasMember("bandwidth") || !v["bandwidth"].IsArray() ||
            !v.HasMember("iot") || !v["iot"].IsArray() ||
            !v.HasMember("source") || !v["source"].IsObject() ||
            !v.HasMember("leader") || !v["leader"].IsString())
            return false;
        report_result result;
        memset(&result.hardware,0,sizeof(Report::hardware_result));
        result.source.setJson(v["source"]);

        Value &val = v["hardware"];

        if( !val.HasMember("cores") || !val["cores"].IsInt() ||
            !val.HasMember("mean_free_cpu") || !val["mean_free_cpu"].IsFloat() ||
            !val.HasMember("var_free_cpu") || !val["var_free_cpu"].IsFloat() ||
            !val.HasMember("memory") || !val["memory"].IsInt64() ||
            !val.HasMember("mean_free_memory") || !val["mean_free_memory"].IsFloat() ||
            !val.HasMember("var_free_memory") || !val["var_free_memory"].IsFloat() ||
            !val.HasMember("disk") || !val["disk"].IsInt64() ||
            !val.HasMember("mean_free_disk") || !val["mean_free_disk"].IsFloat() ||
            !val.HasMember("var_free_disk") || !val["var_free_disk"].IsFloat() ||
            !val.HasMember("lasttime") || !val["lasttime"].IsInt64())
            return false;
        {
            result.hardware.cores = val["cores"].GetInt();
            result.hardware.mean_free_cpu = val["mean_free_cpu"].GetFloat();
            result.hardware.var_free_cpu = val["var_free_cpu"].GetFloat();
            result.hardware.memory = val["memory"].GetInt64();
            result.hardware.mean_free_memory = val["mean_free_memory"].GetFloat();
            result.hardware.var_free_memory = val["var_free_memory"].GetFloat();
            result.hardware.disk = val["disk"].GetInt64();
            result.hardware.mean_free_disk = val["mean_free_disk"].GetFloat();
            result.hardware.var_free_disk = val["var_free_disk"].GetFloat();
            result.hardware.lasttime = val["lasttime"].GetInt64();
        }

        for (auto& v : v["latency"].GetArray()) {
            if( !v.IsObject() ||
                !v.HasMember("target") || !v["target"].IsObject() ||
                !v.HasMember("mean") || !v["mean"].IsFloat() ||
                !v.HasMember("variance") || !v["variance"].IsFloat() ||
                !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
                return false;
            test_result test;
            test.target.setJson(v["target"]);
            test.mean = v["mean"].GetFloat();
            test.variance = v["variance"].GetFloat();
            test.lasttime = v["lasttime"].GetInt64();

            result.latency.push_back(test);
        }

        for (auto& v : v["bandwidth"].GetArray()) {
            if( !v.IsObject() ||
                !v.HasMember("target") || !v["target"].IsObject() ||
                !v.HasMember("mean") || !v["mean"].IsFloat() ||
                !v.HasMember("variance") || !v["variance"].IsFloat() ||
                !v.HasMember("lasttime") || !v["lasttime"].IsInt64())
                return false;
            test_result test;
            test.target.setJson(v["target"]);
            test.mean = v["mean"].GetFloat();
            test.variance = v["variance"].GetFloat();
            test.lasttime = v["lasttime"].GetInt64();

            result.bandwidth.push_back(test);
        }

        for (auto& v : v["iot"].GetArray()) {
            if( !v.IsObject() ||
                !v.HasMember("id") || !v["id"].IsString() ||
                !v.HasMember("desc") || !v["desc"].IsString() ||
                !v.HasMember("latency") || !v["latency"].IsInt())
                return false;
            IoT iot;
            iot.id = string(v["id"].GetString());
            iot.desc = string(v["latency"].GetString());
            iot.latency = v["latency"].GetInt();

            result.iot.push_back(iot);
        }
        result.leader = v["leader"].GetString();
        reports.push_back(result);
    }

    return true;
}

string Report::getString() {
    StringBuffer s;
    rapidjson::Writer<StringBuffer> writer (s);
    doc.Accept (writer);
    std::string str (s.GetString());
    return str;
}