#pragma once

#include "SimEvent.h"
#include "SimEntity.h"
#include "Message.h"
#include <vector>
#include <boost/random.hpp>

using namespace mgsim;

namespace OpinionDynamics{

/* 
The base class for all kinds of agents in opinion dynamics. 
A social agent exchanges opinion with its neigbours, and is also affected by its neigbours.
*/
class SocialAgent : public SimEntity
{
protected:
	OpValue internalOpinion;  		//!< the internal opinion for an agent. [0,1]
	OpValue expressedOpinion;		//!< the opinion that an agent express to public. [0,1]
	
	double suspecible;				//!< 易感性，表示是否容易受到其他人影响。
	double confidenceBound;			//!< 有界置信区间，only opinion within the bound can affect the agent

	double limitflow;				//!< 表示限制流量	

	std::map<SimEntityID, double> friendsOpinions;
	std::set<SimEntityID> followers;

	double delta;

public:

	virtual void Init();
	virtual void execute(SimMsg*);
	virtual void Terminate(SimTime);

	SocialAgent(OpValue inOp, double susp, double opbd = 0.3)
	{
		internalOpinion = inOp;
		suspecible = susp;
		confidenceBound = opbd;
		limitflow = 1.0;
	}

	virtual ~SocialAgent()
	{ 
		friendsOpinions.clear();
		followers.clear();
	}

	//---------- Utilities, suitable for all kinds of agents ------------------------------//
	OpValue getInternalOpinion(){ return internalOpinion; } 
	OpValue getExpressedOpinion(){ return expressedOpinion; } 
	void addFriend(SimEntityID sid);
	void removeFriend(SimEntityID sid);
	void clearAllFriends();

	void addFollower(SimEntityID sid);
	void removeFans(SimEntityID sid);

	void ExpressOpinion2Followers();						
	void RecvFriendOpinion(SimEntityID id, UserOpinionEvent* stc);	

	//---------- Default implementation, can be redefined -------------------------------//
	virtual void FuseOpinion();							//!< adjust opinion by fuse others, HK rule
	virtual void DecideExpressedOpinion();				//!< decide how to express opinion, thinklevel = 0
};

}


