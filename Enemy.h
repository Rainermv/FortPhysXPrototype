#pragma once

#include <NxPhysics.h>

class Enemy
{
public:
	Enemy(void);
	Enemy(float spd, float dam, NxVec3 tar);
	~Enemy(void);

	
	//NxVec3 starting_position;
	//NxVec3 size;
	NxVec3 target;
	NxActor* actor;

	float damage;
	float speed;
};


