#include "SocialAgent.hpp"

#include <iostream>
#include <math.h>
#include <time.h>
#include <string>

using namespace std;
using namespace boost::random;
using namespace OpinionDynamics;

void SocialAgent::Init()
{
    delta = 1;

    //!< send T_Follow messages to its friends
    /*for(auto person : friendsOpinions)
    {
        SimMsg* pmsg = new SimMsg(T_Follow);
        send(person.first, pmsg, 0.1);
    }*/

	ExpressOpinion2Followers();
    //!< 对个体对应的共享区进行修改
	dynamic_cast<AgentsOpinionState*>(getIntraLPState())->add(EntityID(), internalOpinion);

	startTick(delta);

}

void SocialAgent::execute(SimMsg *pmsg)
{
	int ev_type = pmsg->getMsgType();

	switch (ev_type)
	{
		case SIM_ENT_TICKMSG:
		{
			FuseOpinion();
            DecideExpressedOpinion();
            ExpressOpinion2Followers();

            //!< 对个体对应的共享区进行修改
			dynamic_cast<AgentsOpinionState*>(getIntraLPState())->modify(EntityID(), internalOpinion);
			break;
		}
		case T_UserOpinion: 
		{
			RecvFriendOpinion( pmsg->get_src_entityid(), static_cast<UserOpinionEvent*>(pmsg->getSimEvent()));
			break;
		}
        case T_Follow:
        {
            auto fans = pmsg->get_src_entityid();
            auto res = followers.emplace(pmsg->get_src_entityid());
            if( res.second == false )
            {
                SIMDBG(0, "The entity " << fans << " has already been a follower of entity " << EntityID())
            }
            break;
        }
		default: {
			SIMDBG(0, "cannot find the corresponding handler! " << ev_type );
			break;
		}
	}
}

void SocialAgent::Terminate(SimTime ts)
{
	endTick(delta);	
}

void SocialAgent::addFriend(SimEntityID nid)
{
    auto ret = friendsOpinions.emplace(nid, -1);
    if( ret.second == false )
    {
        SIMDBG(0, "The agent " << nid << " is not inserted into friends successfully!");
    }
}

void SocialAgent::removeFriend(SimEntityID nid)
{
    auto ret = friendsOpinions.erase(nid);
    if( ret == 0 )
    {
        SIMDBG(0, "The agent " << nid << " is not " << EntityID() << "'s neigbour!");
    }
}

void SocialAgent::clearAllFriends()
{
    friendsOpinions.clear();
}

void SocialAgent::addFollower(SimEntityID sid)
{
    auto ret = followers.emplace(sid);
    if( ret.second == false )
    {
        SIMDBG(0, "The agent " << sid << " is not inserted into followers successfully!");
    }
}
	
void SocialAgent::removeFans(SimEntityID sid)
{
    auto ret = followers.erase(sid);
    if( ret == 0 )
    {
        SIMDBG(0, "The agent " << sid << " is not " << EntityID() << "'s fan!");
    }
}

//!< Hegselmann-Krause rules
void SocialAgent::FuseOpinion()
{
    double accumOpinion = 0.0;
    int numAffect = 0;

	for(auto& op : friendsOpinions)
    {
        if( op.second >= 0 ) 
        {
            auto diff = op.second - internalOpinion;
            if( abs(diff) < confidenceBound )
            {
                accumOpinion += diff;
                numAffect++;
            }

            op.second = -1;     //!< 表示观点值已被消化
        }
    }

    if( numAffect > 0 )
    {
        internalOpinion += suspecible*accumOpinion/numAffect;
    }
}

void SocialAgent::DecideExpressedOpinion()
{
    expressedOpinion = internalOpinion;
}

void SocialAgent::ExpressOpinion2Followers()
{
    boost::uniform_01<> dist;
    for(auto& dest : followers)
    {
        if( dist(getrngseed()) < limitflow )
        {
            UserOpinionEvent* eo = new UserOpinionEvent(expressedOpinion);
            SimMsg* pmsg = new SimMsg(T_UserOpinion, eo);
            send(dest, pmsg, delta);
        }
    }
}

void SocialAgent::RecvFriendOpinion(SimEntityID id, UserOpinionEvent* epo)
{
    auto it = friendsOpinions.find(id);

    if( it != friendsOpinions.end() )
    {
        it->second = epo->getOpinion();
    }else{
        SIMDBG(0, EntityID() << " receives a message from the stranger " << id );
    }
}

