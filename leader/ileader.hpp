#ifndef I_LEADER_HPP_
#define I_LEADER_HPP_

#include "iagent.hpp"
#include "ileader_storage.hpp"

class ILeader : virtual public IAgent {
public:
    virtual ILeaderStorage* getStorage() = 0;
    virtual bool initSelection(int id) = 0;
    virtual bool calcSelection(Message::node from,int id, bool &res) = 0;
    virtual bool updateSelection(Message::leader_update update) = 0;
    virtual void changeRoles(Message::leader_update update) = 0;
    virtual void stopSelection() = 0;
};

#endif