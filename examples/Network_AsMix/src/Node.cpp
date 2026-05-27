#include "Node.h"

#include <iostream>
#include <math.h>
#include <time.h>
#include <string>

using namespace std;
using namespace boost::random;

void Node::Init()
{
    work_delta = 1;
    comm_delta = 0.1;

    for (auto &i: successors)
    {
        string content{"something"};
        DataPackage *dp  = new DataPackage(nodeid, i, content);
        SimMsg *comm_msg = new SimMsg(T_DataPack, dp);
        send(i, comm_msg, comm_delta);
        //cout << "The node (" << regionid << ", " << nodeid << ") sends a message to node " << i << endl ;
    }

    startTick(work_delta);

    //SimMsg* selfmsg = new SimMsg(T_SelfWork);
    //send(EntityID(), selfmsg, work_delta, 999);
}

void Node::execute(SimMsg *pmsg)
{
    int ev_type = pmsg->getMsgType();

    switch (ev_type)
    {
        case SIM_ENT_TICKMSG:
        {
            CycleWork();
            break;
        }
        case T_SelfWork:
        {
            CycleWork();
            SimMsg *selfmsg = new SimMsg(T_SelfWork);
            send(EntityID(), selfmsg, work_delta, 999);
            break;
        }
        case T_DataPack:
        {
            recvDataPackage(static_cast<DataPackage *>(pmsg->getSimEvent()));
            break;
        }
        default:
        {
            SIMDBG(0, "cannot find the corresponding handler! " << ev_type);
            break;
        }
    }
}

void Node::Terminate(SimTime ts) { endTick(work_delta); }

void Node::CycleWork()
{
    /*for (auto& i : predessors)
    {
        if( tempsrcs.erase(i) == false )
		{
            SIMDBG(0, "Node " << id <<" cannot find a message from the predessor " << i << " at " << now() );
			//SimTime temp = now();
			//temp.SetPriority1(0);
			//if( now() < temp )
			//	cout << "not correct!" << endl;
		}
		
    }*/

    for (auto &i: successors)
    {
        string content{"something"};
        DataPackage *dp = new DataPackage(nodeid, i, content);
        ENSURE(0, dp != nullptr, "cannot be an empty pointer.");
        SimMsg *comm_msg = new SimMsg(T_DataPack, dp);
        send(i, comm_msg, comm_delta);
        //cout << "The node (" << regionid << ", " << nodeid << ") sends a message to node " << i << " at " << now() << endl ;
    }
}

void Node::recvDataPackage(DataPackage *dp) { tempsrcs.emplace(dp->getSource()); }