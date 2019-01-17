#include "Fort.h"


Fort::Fort(void)
{
}

Fort::Fort(NxVec3 pos, NxVec3 siz, float lif)
{
	position = pos;
	//size = NxVec3(5,5,3);
	size = siz;

	actor = NULL;
	trigger = NULL;

	life = lif;
}

Fort::~Fort(void)
{
}
