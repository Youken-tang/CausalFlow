#pragma once

#include "SimEvent.h"
#include "SimEntity.h"
#include "Message.h"
#include "typedefines.hpp"
#include <vector>
#include <boost/random.hpp>

using namespace mgsim;


/* This model is an attempt to mimic the flocking of birds. (The resulting motion also resembles schools of fish.) The flocks that appear in this model are not created or led in any way by special leader birds. Rather, each bird is following exactly the same set of rules, from which flocks emerge.

The birds follow three rules: "alignment", "separation", and "cohesion". 

"Alignment" means that a bird tends to turn so that it is moving in the same direction that nearby birds are moving. 
"Separation" means that a bird will turn to avoid another bird which gets too close. 
"Cohesion" means that a bird will move towards other nearby birds (unless another bird is too close). 

When two birds are too close, the "separation" rule overrides the other two, which are deactivated until the minimum separation is achieved.

The three rules affect only the bird's heading. Each bird always moves forward at the same constant speed.
*/
class BirdShm : public SimEntity
{
protected:
	Vector3D position; //!< the position for a bird;
	Vector3D velocity; //!< the velocity for a bird;
	Vector3D acceleration; //!< the acceleration for a bird;

	float maxSpeed; //!< the maximun speed
	float maxForce; //!< the maximun force

	double seperate_range; //!< 分离规则的距离
	double alignment_range; //!< 对齐规则的距离
	double cohesion_range; //!< 聚合规则的距离


	// Youken 2025.12.12 the vector is useless
	// std::vector<BirdTrace *> neighborbirds; //!< records the birds nearby

	double delta;

public:
	virtual void Init();

	virtual void execute(SimMsg *);

	virtual void Terminate(SimTime);

	BirdShm(Vector3D &pos, Vector3D &vec, double sr, double cr)
	{
		position = pos;
		velocity = vec;
		seperate_range = sr;
		cohesion_range = cr;
	}

	~BirdShm() {}

	void CycleWork();

	void recvBirdTrace(BirdTrace *stc);

    virtual void flock();

    void applyForce(const Vector3D &force);

    // Three Laws that boids follow
	Vector3D Separation(const BoidsState *Boids);

	Vector3D Alignment(const BoidsState *Boids);

	Vector3D Cohesion(const BoidsState *Boids);

	// Vector3D Separation(const vector<BirdTrace*>& Boids);
	// Vector3D Alignment(const vector<BirdTrace*>& Boids);
	// Vector3D Cohesion(const vector<BirdTrace*>& Boids);

	Vector3D &getPosition() { return position; }
    const Vector3D &getPosition() const { return position; }

    Vector3D &getVelocity() { return velocity; }
    const Vector3D &getVelocity() const { return velocity; }
};


