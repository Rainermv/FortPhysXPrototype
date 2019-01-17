#pragma once

#include <NxPhysics.h>

class Fort
{
public:
	Fort(void);
	Fort(NxVec3 pos, NxVec3 siz, float lif);
	~Fort(void);

	NxVec3 position;
	NxVec3 size;

	NxActor* actor;
	NxActor* trigger;

	float life;
};

