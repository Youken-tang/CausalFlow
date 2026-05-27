#include "BehaviorGameAgent.hpp"

#include <iostream>
#include <math.h>

using namespace std;
using namespace boost::random;
using namespace OpinionDynamics;


/// @brief Fuse opinion based on H-K rule. Compute mean opinoin of friends
void BehaviorGameAgent::FuseOpinion()
{
    double accumOpinion = 0.0;
    int numAffect = 0;
    estFriendsOpinion = 0.0;
    estPublicOpinion = 0.0;
    int numExpressedFriends = 0;
    int numRecvedPubOpinion = 0;

    boost::random::uniform_01<> samplefriend;
    auto numintfrds = percentFriends*numinteractions;
    for(auto& op : friendsOpinions)
    {
        if( samplefriend(getrngseed()) < numintfrds/friendsOpinions.size() )
        {
            if( op.second >= 0 ) 
            {
                auto diff = op.second - internalOpinion;
                if( abs(diff) < confidenceBound )
                {
                    accumOpinion += diff;
                    numAffect++;
                }
                estFriendsOpinion += op.second;
                numExpressedFriends++;

                op.second = -1;     //!< 表示观点值已被消化
            }
        }
    }


    int totalpublic = percentPublic*numinteractions;
    boost::random::uniform_int_distribution<> strgdist{0, numpersons-1};

    auto ind = strgdist(getrngseed());
    while (numRecvedPubOpinion < totalpublic)
    {
        auto lpid = getLPID(ind);

        AgentsOpinionState* paoS = dynamic_cast<AgentsOpinionState*>(getSharedState()->getReadSharedState()[lpid]);
        auto op = paoS->databuffer.find(ind);
        if( op != paoS->databuffer.end() )
        {
            if( op->second >= 0 ) 
            {
                auto diff = op->second - internalOpinion;
                if( abs(diff) < confidenceBound )
                {
                    accumOpinion += diff;
                    numAffect++;
                }
                estPublicOpinion += op->second;
                numRecvedPubOpinion++;
            }
        }
        ind = strgdist(getrngseed());
    }
    
    if( numAffect > 0 )
    {
        internalOpinion += suspecible*accumOpinion/numAffect;
        ENSURE(0, internalOpinion > 0.0 && internalOpinion < 1.0, EntityID() << "'s opinion is " << internalOpinion);
    }

    if( numExpressedFriends > 0 )
    {
        estFriendsOpinion /= numExpressedFriends;
    }

    if( numRecvedPubOpinion > 0 )
    {
        estPublicOpinion /= numRecvedPubOpinion;
    }

}

void BehaviorGameAgent::DecideExpressedOpinion()
{
    switch (thinklevel)
    {
        case 0:
        {
            expressedOpinion = internalOpinion;
            break;
        }
        case 1:
        {
            expressedOpinion = alpha*internalOpinion + beta*estPublicOpinion + gamma*estFriendsOpinion;
            break;
        }
        case 2:
        {
            expressedOpinion = alpha*internalOpinion + beta*(1-gamma)*estPublicOpinion + gamma*(1+beta)*estFriendsOpinion;
            break;
        }
        default:
        {
            SIMDBG(0, "Undefined mode!");
            break;
        }
    }

}