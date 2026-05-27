#include "BirdShm.h"
#include "SyncConTMtbb.h"

#include <iostream>
#include <math.h>
#include <time.h>
#include <string>

using namespace std;
using namespace boost::random;

extern const double Length;
extern const double Width;
extern const double Height;

void BirdShm::Init()
{
	acceleration = std::move( Vector3D(0, 0, 0) );
    
    maxSpeed = 3.5;
    maxForce = 0.5;

	seperate_range = 20;
	alignment_range = 50;
	cohesion_range = 50;
	delta = 1;
	
	//!< 将数据写入LP共享内存区
	BirdTrace bt(EntityID(), position, velocity);
	dynamic_cast<BoidsState*>(getIntraLPState())->add(EntityID(), bt);

	startTick(delta);

}

void BirdShm::execute(SimMsg *pmsg)
{
	int ev_type = pmsg->getMsgType();

	switch (ev_type)
	{
		case SIM_ENT_TICKMSG:
		{
			CycleWork();

			//!< 将数据写入LP共享内存区
			BirdTrace bt(EntityID(), position, velocity);
			dynamic_cast<BoidsState*>(getIntraLPState())->modify(EntityID(), bt);

			break;
		}
		default: {
			SIMDBG(0, "cannot find the corresponding handler! " << ev_type );
			break;
		}
	}
}

void BirdShm::Terminate(SimTime ts)
{
	endTick(delta);	
}

void BirdShm::CycleWork()
{
	flock();

    //To make the slow down not as abrupt
    acceleration *= 0.4;

    // Update velocity
    velocity += acceleration;

    // Limit speed
    velocity.limit(maxSpeed);
	position += velocity*delta;

    // Reset accelertion to 0 each cycle
    acceleration *= 0.0;

	//!< 越界时修正位置
	if (position.x < 0)    position.x += Length;
    if (position.y < 0)    position.y += Width;
	if (position.z < 0)    position.z += Height;
    if (position.x > Length) position.x -= Length;
    if (position.y > Width) position.y -= Width;
	if (position.z > Height) position.z -= Height;

	//BirdTrace* birdt = new BirdTrace(id, position, velocity);
	//SimMsg* posmsg = new SimMsg(T_BirdTrace, birdt);
	//post("BirdTrace", posmsg, 0.1);

	//neighborbirds.clear();
}

void BirdShm::recvBirdTrace(BirdTrace* stc)
{
	// neighborbirds.push_back(stc);
}

// 决定bird的移动方向
void BirdShm::flock()
{
    Vector3D sep, ali, coh;
    for( auto& neighborbirds : getSharedState()->getReadSharedState())
    {
        sep += Separation(dynamic_cast<BoidsState*>(neighborbirds) );
        ali += Alignment(dynamic_cast<BoidsState*>(neighborbirds));
        coh += Cohesion(dynamic_cast<BoidsState*>(neighborbirds) );
    }
   
   	// Arbitrarily weight these forces
    sep *= 1.5;
    ali *= 1.0; // Might need to alter weights for different characteristics
    coh *= 1.0;
    
	// Add the force vectors to acceleration
    applyForce(sep);
    applyForce(ali);
    applyForce(coh);
}

// Adds force to current acceleration
void BirdShm::applyForce(const Vector3D& force)
{
	acceleration = acceleration + force;
}

// Separation
// Keeps boids from getting too close to one another
Vector3D BirdShm::Separation(const BoidsState* boids)
{
    // Distance of field of vision for separation between boids
    Vector3D steer(0, 0, 0);
    int count = 0;
    // For every boid in the system, check if it's too close
    for ( auto& iter : boids->databuffer ) 
	{
        // Calculate distance from current boid to boid we're looking at
		auto diff = iter.second.getPosition() - position;
		auto d = diff.length();
		// If this is a fellow boid and it's too close, move away from it
        if ((d > 0) && (d < seperate_range)) 
		{
            diff.normalize();
            diff /= d;      // Weight by distance
            steer += diff;
            count++;
		}
    }
    // Adds average difference of location to acceleration
    if (count > 0)
        steer /= count;

    if (steer.length() > 0) {
        // Steering = Desired - Velocity
        steer.normalize();
		steer *= maxSpeed;
		steer -= velocity;
		steer.limit(maxForce);
    }

    return steer;
}

// Alignment
// Calculates the average velocity of boids in the field of vision and
// manipulates the velocity of the current bird in order to match it
Vector3D BirdShm::Alignment(const BoidsState* boids)
{

    Vector3D sum(0, 0, 0);
    int count = 0;
    for (auto& iter : boids->databuffer) 
	{
        float d = (position - iter.second.getPosition()).length();
        if ((d > 0) && (d < alignment_range)) // 0 < d < 50
		{
			sum += iter.second.getVelocity();
            count++;
        }
    }
    // If there are boids close enough for alignment...
    if (count > 0) 
	{
		sum /= count;			// Divide sum by the number of close boids (average of velocity)
		sum.normalize();		// Turn sum into a unit vector, and
		sum *= maxSpeed;		// Multiply by maxSpeed

        // Steer = Desired - Velocity
        Vector3D steer;
        steer = sum - velocity;				//sum = desired(average)
		steer.limit(maxForce);
        return steer;

    } else {
		Vector3D tmp;
        return tmp;
    }
}

// Cohesion
// Finds the average location of nearby boids and manipulates the
// steering force to move in that direction.
Vector3D BirdShm::Cohesion(const BoidsState* boids)
{
    
    Vector3D sum(0, 0, 0);
    int count = 0;
    for (auto& iter : boids->databuffer) 
	{
        float d = ( position - iter.second.getPosition() ).length();
        if ((d > 0) && (d < cohesion_range)) 
		{
			sum += iter.second.getPosition();
            count++;
        }
    }
    if (count > 0) 
	{
		sum /= count;
		return sum;

        //sum.divScalar(count);
        //return std::move( seek(sum) );
    } else 
	{
        Vector3D temp(0, 0, 0);
        return temp;
    }
}
/*
// Limits the maxSpeed, finds necessary steering force and
// normalizes vectors
Vector3D& BirdShm::seek(const Vector3D& v)
{
    Vector3D* desired = new Vector3D();
    desired.subVector(v);  // A vector pointing from the location to the target
    // Normalize desired and scale to maximum speed
    desired.normalize();
    desired.mulScalar(maxSpeed);
    // Steering = Desired minus Velocity
    acceleration.subTwoVector(desired, velocity);
    acceleration.limit(maxForce);  // Limit to maximum steering force
    return acceleration;
}

*/


