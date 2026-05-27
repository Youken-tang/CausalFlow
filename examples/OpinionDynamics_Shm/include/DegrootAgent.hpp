#pragma once

#include "SocialAgent.hpp"

namespace OpinionDynamics
{

class DegrootAgent : public SocialAgent{

protected:
    
public:

	DegrootAgent(OpValue inOp, double susp, double opbd = 0.3)
    : SocialAgent(inOp, susp, opbd)
	{
		
	}

	virtual ~DegrootAgent(){ }

	//---------- Default implementation, can be redefined -------------------------------//
	virtual void FuseOpinion();							//!< adjust opinion by fuse others, Degroot rule

};


} // namespace name


