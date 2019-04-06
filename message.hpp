#ifndef MESSAGE_HPP_
#define MESSAGE_HPP_

#include <vector>
#include <string>
#include "rapidjson/document.h"
#include "report.hpp"

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
    enum Command {GET, SET, HELLO, NODELIST, MNODELIST, UPDATE, START};
    /**
     * possible arguments for the messages
    */
    enum Argument {NONE, NODES, MNODES, REPORT, POSITIVE, NEGATIVE, TOKEN, IPERF, ESTIMATE, LATENCY, BANDWIDTH};

    Message();
    ~Message();

    /**
     * clear the message in order to rebuild it
    */
    void Clear();

    /**
     * fill the message with a json string
    */
    bool parseJson(char* json);

    /**
     * build the json string to be getted from getString()
    */
    void buildString();
    /**
     * get the json string of this message
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
     * set the data as a vector of strings
     * @param strings
    */
    void setData(std::vector<std::string> strings);
    /**
     * set the data as a pair: a string and a vector of strings
     * @param string
     * @param strings
    */
    void setData(std::string string, std::vector<std::string> strings);
    /**
     * set the data as a pair of vectors of strings
     * @param stringsA
     * @param stringsB
    */
    void setData(std::vector<std::string> stringsA, std::vector<std::string> stringsB);
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
     * get the data expecting a vector of strings
     * @param strings
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(std::vector<std::string>& strings);
    /**
     * get the data expecting a pair: a string and a vector of strings
     * @param string
     * @param strings
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(std::string& string, std::vector<std::string>& strings);
    /**
     * get the data expecting a pair of vectors of strings
     * @param stringsA
     * @param stringsB
     * @return true if successful, else failed to interpreter the data or other errors
    */
    bool getData(std::vector<std::string>& stringsA, std::vector<std::string>& stringsB);
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