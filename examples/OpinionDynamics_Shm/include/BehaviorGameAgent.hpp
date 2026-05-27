#pragma once

#include "SocialAgent.hpp"

namespace OpinionDynamics
{

class BehaviorGameAgent : public SocialAgent{

protected:
    int thinklevel;   			//!< 思考层次
	int numinteractions; 		//!< 交互次数
	double percentFriends, percentPublic;
	long numpersons;

	/// @brief 不同收益对agent决策影响的权重，self, public, friend
	double alpha, beta, gamma;
	OpValue estPublicOpinion;
	OpValue estFriendsOpinion;

public:

	BehaviorGameAgent(double inOp, double susp, long nps, double confbound = 0.3, double a = 0.5, double b = 0.3, double c = 0.2)
    : SocialAgent(inOp, susp, confbound)
	{
		alpha = a, beta = b, gamma = c;
		numpersons = nps;

		numinteractions = 10;
		percentFriends = 0.5, percentPublic = 0.5;

		//boost::uniform_01<> dist;
		auto prob = drand48();
		if( prob < 0.9 )
		{
			thinklevel = 0;
		}else if( prob < 0.95 )
		{
			thinklevel = 1;
		}else{
			thinklevel = 2;
		}

		estPublicOpinion = 0.0;
		estFriendsOpinion = 0.0;
	}

	virtual ~BehaviorGameAgent(){ }

	//---------- Default implementation, can be redefined -------------------------------//							
	virtual void FuseOpinion();							//!< fuse opinion
	virtual void DecideExpressedOpinion();				//!< decide how to express opinion, thinklevel = 0

};


} // namespace name