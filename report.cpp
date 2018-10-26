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

    
    doc.AddMember("latency", arr, doc.GetAllocator());
}

void Report::setBandwidth(vector<test_result> bandwidth) {

    Value arr(kArrayType);
    doc.RemoveMember("bandwidth");

    
    doc.AddMember("bandwidth", arr, doc.GetAllocator());

}

bool Report::getHardware(hardware_result& hardware) {
    if( !this->doc.HasMember("hardware") && !this->doc["hardware"].IsObject())
        return false;

    Value &val = doc["hardware"];
    
    if( !val.HasMember("cores") && !val["cores"].IsInt() &&
        !val.HasMember("free_cpu") && !val["free_cpu"].IsFloat() &&
        !val.HasMember("memory") && !val["memory"].IsInt() &&
        !val.HasMember("free_memory") && !val["free_memory"].IsInt() &&
        !val.HasMember("disk") && !val["disk"].IsInt() &&
        !val.HasMember("free_disk") && !val["free_disk"].IsInt())
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
    return true;
}

bool Report::getBandwidth(vector<test_result>& bandwidth) {
    return true;
}