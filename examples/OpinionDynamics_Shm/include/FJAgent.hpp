#pragma once

#include "SocialAgent.hpp"

namespace OpinionDynamics
{

class FJAgent : public SocialAgent{

protected:
    OpValue innateOpinion;   //!< 预设观点值

public:

	FJAgent(OpValue initop, OpValue inOp, double susp, double opbd = 0.3)
    : SocialAgent(inOp, susp, opbd)
	{
		innateOpinion = initop;
	}

	virtual ~FJAgent(){ }

	//---------- Default implementation, can be redefined -------------------------------//
	virtual void FuseOpinion();							//!< adjust opinion by fuse others, Degroot rule

};


} // namespace name


