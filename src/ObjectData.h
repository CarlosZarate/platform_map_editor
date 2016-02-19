
#pragma once

#include "Urho3D/Core/Object.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Math/Vector2.h"
#include "Urho3D/Scene/Component.h"

using namespace Urho3D;

class ObjectData: public Component
{
    URHO3D_OBJECT(ObjectData, Component);
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

