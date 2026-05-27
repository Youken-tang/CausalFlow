#pragma once

#include "SimEvent.h"
#include "SimEntity.h"
#include "Message.h"
#include <vector>
#include <unordered_set>
#include <boost/random.hpp>

using namespace mgsim;


/* This model is an attempt to mimic a simple network simulation. 
	The model has some regions, and each region has a number of nodes.
	Every node will randomly send messages to its neighbours. 
	Its neighbours may reside in the same region or reside in the nearby region.
    
	The main goal of this model is to test the asynchronous time management algorithm.
*/
class Node : public SimEntity
{
protected:
    SimEntityID nodeid; //!< the id of the node;
    LPID regionid;
    unordered_set<SimEntityID> successors; //!< 后续邻居节点，当前节点发送消息的目标
    unordered_set<SimEntityID> predessors; //!< 前序邻居节点，以当前节点作为目标节点。
    unordered_set<SimEntityID> tempsrcs;   //!< 临时存储发送到当前节点的节点ID

    double work_delta; //!< 工作周期时延
    double comm_delta; //!< 消息通信时延

public:
    virtual void Init();

    virtual void execute(SimMsg *);

    virtual void Terminate(SimTime);

    Node(SimEntityID nid, LPID rid) : nodeid(nid), regionid(rid) {}

    ~Node() { successors.clear(); }

    void CycleWork();

    void recvDataPackage(DataPackage *dp);

    void addsuccessor(SimEntityID s) { successors.emplace(s); }
    void addpredessor(SimEntityID p) { predessors.emplace(p); }
    SimEntityID nodeID() { return nodeid; }
    LPID &regionID() { return regionid; }
};