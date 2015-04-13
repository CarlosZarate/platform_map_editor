
#pragma once

#include "Object.h"
#include "Context.h"
#include "Vector2.h"
#include "Component.h"

using namespace Urho3D;

class ObjectData: public Component
{
    OBJECT(ObjectData);
public:
    ObjectData(Context* context);
    static void RegisterObject(Context* context);
    Vector2 position;
    Vector2 p1;
    Vector2 p2;
    String type;
    String object_orientation;
    String Code;
    void SetPostion(Vector2 pos);
private:

};

