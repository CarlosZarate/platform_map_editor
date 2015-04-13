
#include "PlatformData.h"


PlatformData::PlatformData(Context* context): Component(context)
{

}

void PlatformData::RegisterObject(Context* context)
{
	context->RegisterFactory<PlatformData>();
}

