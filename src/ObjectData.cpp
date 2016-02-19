
#include "ObjectData.h"
#include "Urho3D/Scene/Node.h"


ObjectData::ObjectData(Context* context): Component(context)
{

}

void ObjectData::RegisterObject(Context* context)
{
	context->RegisterFactory<ObjectData>();
}

void ObjectData::SetPostion(Vector2 pos)
{
    position = pos;
    node_->SetPosition2D(position);
}

