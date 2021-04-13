#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

class Report;
/**
 * The actual message builder for communications
*/
class Message {
public:

    /**
     * possible types for the messages
    */
    enum Type {REQUEST, NOTIFY, RESPONSE, MREQUEST, MNOTIFY, MRESPONSE};
    /**
     * possible commands for the messages
    */
    enum Command {GET, SET, HELLO, MHELLO, NODELIST, MNODELIST, UPDATE, START, SELECTION_INIT, SELECTION_START, SELECTION, SELECTION_END};
    /**
     * possible arguments for the messages
    */
    enum Argument {NONE, NODES, MNODES, REPORT, POSITIVE, NEGATIVE, IPERF, ESTIMATE, LATENCY, BANDWIDTH, ROLES};

    typedef struct node {
        std::string id;
        std::string ip;
        std::string port;
        node() {
            id = "";
            ip = "";
            port = "";
        }
        node(std::string id, std::string ip, std::string port) {
            this->id = id;
            this->ip = ip;
            this->port = port;
        }

        bool operator==(node const &a) const {
            return (a.id == this->id) && (a.ip == this->ip) && (a.port == this->port);
        }

        rapidjson::Value getJson(rapidjson::Document::AllocatorType& allocator) {
            rapidjson::Value obj(rapidjson::kObjectType);
            rapidjson::Value id(this->id.c_str(), allocator);
            rapidjson::Value ip(this->ip.c_str(), allocator);
            rapidjson::Value port(this->port.c_str(), allocator);

            obj.AddMember("id", id, allocator);
            obj.AddMember("ip", ip, allocator);
            obj.AddMember("port", port, allocator);
            return obj;
        }
        bool setJson(rapidjson::Value &v) {
            if( !v.HasMember("id") || !v["id"].IsString() ||
                !v.HasMember("ip") || !v["ip"].IsString() ||
                !v.HasMember("port") || !v["port"].IsString() )
            return false;

            this->id = v["id"].GetString();
            this->ip = v["ip"].GetString();
            this->port = v["port"].GetString();
            return true;
        }
    }node;

    typedef struct leader_update {
        std::vector<node> selected;
        float cost;
        int changes;
        int id;
        leader_update() {
            selected = std::vector<node>();
            cost = 0;
            changes = 0;
            id = 0;
        }
        leader_update(std::vector<node> selected, float cost, int changes, int id) {
            this->selected = selected;
            this->cost = cost;
            this->changes = changes;
            this->id = id;
        }
        bool empty() {
            if(cost = 0 && id == 0 && changes == 0 && selected.empty()) {
                return true;
            }
            return false;
        }

        rapidjson::Value getJson(rapidjson::Document::AllocatorType& allocator) {
            rapidjson::Value obj(rapidjson::kObjectType);
            rapidjson::Value selected(rapidjson::kArrayType);
            for(auto node : this->selected) {
                selected.PushBack(node.getJson(allocator), allocator);
            }
            rapidjson::Value cost(this->cost);
            rapidjson::Value changes(this->changes);
            rapidjson::Value id(this->id);

            obj.AddMember("selected", selected, allocator);
            obj.AddMember("cost", cost, allocator);
            obj.AddMember("changes", changes, allocator);
            obj.AddMember("id",id,allocator);
            return obj;
        }
        bool setJson(rapidjson::Value &v) {
            if( !v.HasMember("selected") || !v["selected"].IsArray() ||
                !v.HasMember("cost") || !v["cost"].IsFloat() ||
                !v.HasMember("changes") || !v["changes"].IsInt() ||
                !v.HasMember("id") || !v["id"].IsInt() )
            return false;

            this->selected.clear();
            for (auto& e : v["selected"].GetArray()) {
                node node;
                if(!node.setJson(e))
                    return false;
                this->selected.push_back(node);
            }

            this->cost = v["cost"].GetFloat();
            this->changes = v["changes"].GetInt();
            this->id = v["id"].GetInt();
            return true;
        }
        std::string getString() {
            rapidjson::StringBuffer s;
            rapidjson::Writer<rapidjson::StringBuffer> writer (s);
            rapidjson::Document doc;
            doc.SetObject();
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            doc.AddMember("leader_update", this->getJson(allocator), allocator);
            doc.Accept (writer);
            std::string str (s.GetString());
            return str;
        }
    }leader_update;

    Message();
    ~Message();

    /**
     * clear the message in order to rebuild it
    */
    void Clear();

    /**
     * fill the message with a json node
    */
    bool parseJson(char* json);

    /**
     * build the json node to be getted from getnode()
    */
    void buildString();
    /**
     * get the json node of this message
    */
    std::string getString();

    void setSender(Message::node s);
    void setType(Type t);
    void setCommand(Command c);
    void setArgument(Argument a);

    Message::node getSender();
    Type getType();
    Command getCommand();
    Argument getArgument();

    /**
     * set the data as an integer
     * @param i
    */
    void setData(int i);
    /**
     * set the data as a float
     * @param f
    */
    void setData(float f);
    /**
     * set the data as a node
     * @param nodes
    */
    void setData(node node);
    /**
     * set the data as a vector of nodes
     * @param nodes
    */
    void setData(std::vector<node> nodes);
    /**
     * set the data as a pair: a node and a vector of nodes
     * @param node
     * @param nodes
    */
    void setData(node node, std::vector<Message::node> nodes);
    /**
     * set the data as a pair of vectors of nodes
     * @param nodesA
     * @param nodesB
    */
    void setData(std::vector<node> nodesA, std::vector<node> nodesB);
    /**
     * set the data as a report
     * @param report
    */
    void setData(Report& report);
    /**
     * set the data as leader_update
     * @param update
    */
    void setData(leader_update& update);

    /**
     * get the data expecting an integer
     * @param i
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(int& i);
    /**
     * get the data expecting a float
     * @param f
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(float& f);
    /**
     * get the data expecting an integer
     * @param i
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(node& node);
    /**
     * get the data expecting a vector of nodes
     * @param nodes
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(std::vector<node>& nodes);
    /**
     * get the data expecting a pair: a node and a vector of nodes
     * @param node
     * @param nodes
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(node& node, std::vector<Message::node>& nodes);
    /**
     * get the data expecting a pair of vectors of nodes
     * @param nodesA
     * @param nodesB
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(std::vector<node>& nodesA, std::vector<node>& nodesB);
    /**
     * get the data expecting a report
     * @param report
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(Report& report);
    /**
     * get the data expecting a leader_update
     * @param update
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(leader_update& update);

private:

    /**
     * the type of the message
    */
    Type type;
    /**
     * the command of the message
    */
    Command command;
    /**
     * the argument of the message
    */
    Argument argument;

    Message::node sender;
    /**
     * the rapidjson document that holds the message
    */
    rapidjson::Document doc;

    /**
     * the rapidjson value that holds the data of the message
    */
    rapidjson::Value data;

};

#endif