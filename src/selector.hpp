#ifndef SELECTOR_HPP_
#define SELECTOR_HPP_

#include "message.hpp"
#include "readproc.hpp"
#include "ileader.hpp"

#include <mutex>
#include <thread>

class Selector {
public:
    Selector(ILeader *leader);
    ~Selector();

    enum Status{FREE, READY, STARTED, RECEIVED, CHANGING};

    //init the selection
    //return true if we are free to start or we can still abort
    //the greater id wins when multiple node want to start
    virtual bool initSelection(int id);
    
    //start the generation of the selected leaders
    //return true if no error
    //res is returned with false when the program decide not to generate the selection
    virtual bool calcSelection(Message::node from, int id, bool &res);

    //save a selection generated externally
    virtual bool updateSelection(Message::leader_update update);

    //stop the selection process if one is in process
    virtual void stopSelection();

    //check if a selections is needed
    //if doit is true start the selection anyway
    virtual bool checkSelection(bool doit = false);

protected:

    virtual Message::leader_update selection(int id);
    virtual void startSelection();

    std::vector<Message::leader_update> updates;

    std::thread selectionThread;
    std::mutex selectionMutex;

    Status status;
    int id;

    ReadProc *clusterProc;
    std::mutex clusterMutex;

    ILeader *parent;
    Sleeper sleeper;
};

#endif