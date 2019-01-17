#include "Enemy.h"


Enemy::Enemy(void)
{
}


Enemy::Enemy(float spd, float dam, NxVec3 tar)
{
	
	target = tar;
	actor = NULL;

	speed = spd;
	damage = dam;
}

Enemy::~Enemy(void)
{
}
