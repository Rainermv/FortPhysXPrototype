#include "Sensor.h"


Sensor::Sensor(void)
{
}


Sensor::~Sensor(void)
{
}

void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
{
	if (status & NX_TRIGGER_ON_ENTER)
	{
		NxActor& triggerActor = triggerShape.getActor();
		NxActor& actor = otherShape.getActor();
	}

	if (status & NX_TRIGGER_ON_LEAVE)
	{
		NxActor& triggerActor = triggerShape.geActor();
		NxActor& actor = otherShape.getActor();
	}
}

	

}
