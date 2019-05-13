#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <vector>
#include <string>
#include "rapidjson/document.h"

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
    enum Command {GET, SET, HELLO, MHELLO, NODELIST, MNODELIST, UPDATE, START};
    /**
     * possible arguments for the messages
    */
    enum Argument {NONE, NODES, MNODES, REPORT, POSITIVE, NEGATIVE, TOKEN, IPERF, ESTIMATE, LATENCY, BANDWIDTH};

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

        rapidjson::Value getJson(rapidjson::Document::AllocatorType& allocator) {
            rapidjson::Value obj(rapidjson::kObjectType);
            rapidjson::Value id(this->id.c_str(), allocator);
            rapidjson::Value ip(this->ip.c_str(), allocator);
            rapidjson::Value port(this->port.c_str(), allocator);

            obj.AddMember("id", ip, allocator);
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

    void setType(Type t);
    void setCommand(Command c);
    void setArgument(Argument a);

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