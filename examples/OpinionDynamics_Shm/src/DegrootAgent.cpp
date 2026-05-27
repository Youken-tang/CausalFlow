#include "DegrootAgent.hpp"

#include <iostream>
#include <math.h>

using namespace std;
using namespace boost::random;
using namespace OpinionDynamics;


void DegrootAgent::FuseOpinion()
{
    double accumOpinion = 0.0;
    int numAffect = 0;

	for(auto& op : friendsOpinions)
    {
        if( op.second >= 0 ) 
        {
            accumOpinion += op.second;
            numAffect++;
            op.second = -1;     //!< 表示观点值已被消化
        }
    }

    if( numAffect > 0 )
    {
        internalOpinion = (1-suspecible)*internalOpinion + suspecible*accumOpinion/numAffect;
    }

}