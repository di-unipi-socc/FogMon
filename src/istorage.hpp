#ifndef ISTORAGE_HPP_
#define ISTORAGE_HPP_

#include <sqlite3.h>
#include <string>
#include <vector>
#include "message.hpp"
#include "report.hpp"
#include "ithing.hpp"

/**
 * An sqlite3 abstract class for the database
*/
class IStorage {

public:
    virtual ~IStorage();

    /**
     * open the database
     * @param path the location of the database
    */
    void open(std::string path);
    /**
     * close the database
    */
    void close();

    /**
     * get the vector of nodes
     * @return
    */
    virtual std::vector<Message::node> getNodes() = 0;

    /**
     * get the state of the hardware saved
     * @return 
    */
    virtual Report::hardware_result getHardware() = 0;
    /**
     * get a vector of latency test
     * @return a vector of tests for the latency
    */
    virtual std::vector<Report::test_result> getLatency(int64_t last = 0, int sensitivity = 10) = 0;
    /**
     * get a vector of bandwidth test
     * @return a vector of tests for the bandwidth
    */
    virtual std::vector<Report::test_result> getBandwidth(int64_t last = 0, int sensitivity = 10) = 0;

    virtual void saveState() = 0;

    /**
     * save a latency test
     * @param ip the destination ip for the test
     * @param ms the result of the test in milliseconds
    */
    virtual void saveLatencyTest(Message::node node, int ms) = 0;
    /**
     * save a bandwidth test
     * @param ip the destination ip for the test
     * @param kbps the result of the test in kilobit per seconds
     * @param state the state for the bandwidth measurement routine
    */
    virtual void saveBandwidthTest(Message::node node, float kbps, int state) = 0;
    /**
     * save a hardware test
     * @param hardware the result of the test
    */
    virtual void saveHardware(Report::hardware_result hardware) = 0;

    /**
     * add and reset the nodes in the list
     * @param nodes a vector of ips
    */
    virtual void refreshNodes(std::vector<Message::node> nodes) = 0;
    /**
     * add and remove nodes
     * @param add a vector of ips to be added
     * @param rem a vector of ips to be removed
    */
    virtual void updateNodes(std::vector<Message::node> add, std::vector<Message::node> rem) = 0;
    
    /**
     * a function used to get the least recently tested node for the latency test
     * @param num the max number of nodes to get
     * @param seconds only the nodes with the test older than seconds are taken in consideration
     * @return a vector of ips representing the selected nodes
    */
    virtual std::vector<Message::node> getLRLatency(int num, int seconds) = 0;
    /**
     * a function used to get the least recently tested node for the bandwidth test
     * @param num the max number of nodes to get
     * @param seconds only the nodes with the test older than seconds are taken in consideration
     * @return a vector of ips representing the selected nodes
    */
    virtual std::vector<Message::node> getLRBandwidth(int num, int seconds) = 0;

    /**
     * the list of iots saved
     * @return the list of iots saved
    */
    virtual std::vector<Report::IoT> getIots() = 0;

    /**
     * save a IoT device
     * @param iot the device
    */
    virtual void addIot(IThing *iot) = 0;

    /**
     * get the last state of the bandwidth test
     * @param ip the ip of the node for the test
     * @param last the last bandwidth test against the ip
     * @return a negative value on fail (-1) else the test
    */
    virtual int getTestBandwidthState(Message::node node, Report::test_result &last) = 0;

    /**
     * set the ip to filter the localhost
    */
    virtual void setFilter(std::string ip) = 0;

    int64_t getTime();

protected:

    /**
     * create the tables for this database
    */
    virtual void createTables() = 0;

    static int getHardwareCallback(void *R, int argc, char **argv, char **azColName);
    static int getTestCallback(void *R, int argc, char **argv, char **azColName);
    static int VectorNodeCallback(void *vec, int argc, char **argv, char **azColName);
    static int VectorIntCallback(void *vec, int argc, char **argv, char **azColName);
    static int VectorIoTCallback(void *vec, int argc, char **argv, char **azColName);

    /**
     * the sqlite3 connection
    */
    sqlite3 *db;
};



#endif