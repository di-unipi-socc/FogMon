#ifndef I_PARENT_MASTER_HPP_
#define I_PARENT_MASTER_HPP_

#include "inode.hpp"
#include "imaster_storage.hpp"

class IParentMaster : virtual public INode {
public:
    virtual IMasterStorage* getStorage() = 0;
};

#endif