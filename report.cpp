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
        
        Value ip(test.ip.c_str(), allocator);
        Value hw(kObjectType);
        Value lt(kArrayType);
        Value bw(kArrayType);
        Value th(kArrayType);

        {
            Report::hardware_result &hardware = test.hardware;
            
        
            Value cores(hardware.cores);
            Value free_cpu(hardware.free_cpu);
            Value memory(hardware.memory);
            Value free_memory(hardware.free_memory);
            Value disk(hardware.disk);
            Value free_disk(hardware.free_disk);

            hw.AddMember("cores", cores, doc.GetAllocator());
            hw.AddMember("free_cpu", free_cpu, doc.GetAllocator());
            hw.AddMember("memory", memory, doc.GetAllocator());
            hw.AddMember("free_memory", free_memory, doc.GetAllocator());
            hw.AddMember("disk", disk, doc.GetAllocator());
            hw.AddMember("free_disk", free_disk, doc.GetAllocator());
        }
        
        for(auto testLt : test.latency) {
            Value target(testLt.target.c_str(), allocator);
            Value mean(testLt.mean);
            Value variance(testLt.variance);
            Value lasttime(testLt.lasttime);
            Value obj(kObjectType);
            obj.AddMember("target",target, allocator);
            obj.AddMember("mean",mean, allocator);
            obj.AddMember("variance",variance, allocator);
            obj.AddMember("lasttime",lasttime, allocator);

            lt.PushBack(obj, allocator);
        }

        for(auto testBw : test.bandwidth) {
            Value target(testBw.target.c_str(), allocator);
            Value mean(testBw.mean);
            Value variance(testBw.variance);
            Value lasttime(testBw.lasttime);
            Value obj(kObjectType);
            obj.AddMember("target",target, allocator);
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
        
        obj.AddMember("ip",ip, allocator);
        obj.AddMember("hardware",hw, allocator);
        obj.AddMember("latency",lt, allocator);
        obj.AddMember("bandwidth",bw, allocator);
        obj.AddMember("iot",th, allocator);

        arr.PushBack(obj, allocator);
    }
    
    doc.AddMember("reports", arr, doc.GetAllocator());
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
            !v.HasMember("latency") || !v["mean"].IsArray() ||
            !v.HasMember("bandwidth") || !v["bandwidth"].IsArray() ||
            !v.HasMember("iot") || !v["iot"].IsArray() ||
            !v.HasMember("ip") || !v["ip"].IsString())
            return false;
        report_result result;
        result.ip = string(v["ip"].GetString());

        Value &val = v["hardware"];

        if( !val.HasMember("cores") || !val["cores"].IsInt() ||
            !val.HasMember("free_cpu") || !val["free_cpu"].IsFloat() ||
            !val.HasMember("memory") || !val["memory"].IsInt() ||
            !val.HasMember("free_memory") || !val["free_memory"].IsInt() ||
            !val.HasMember("disk") || !val["disk"].IsInt() ||
            !val.HasMember("free_disk") || !val["free_disk"].IsInt())
            return false;
        
        {
            result.hardware.cores = val["cores"].GetInt();
            result.hardware.free_cpu = val["free_cpu"].GetFloat();
            result.hardware.memory = val["memory"].GetInt();
            result.hardware.free_memory = val["free_memory"].GetInt();
            result.hardware.disk = val["disk"].GetInt();
            result.hardware.free_disk = val["free_disk"].GetInt();
        }

        for (auto& v : v["latency"].GetArray()) {
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

            result.latency.push_back(test);
        }

        for (auto& v : v["bandwidth"].GetArray()) {
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

        reports.push_back(result);
    }

    return true;
}