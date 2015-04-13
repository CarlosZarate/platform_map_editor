
#pragma once

#include "Object.h"
#include "Context.h"
#include "Vector2.h"
#include "Component.h"

using namespace Urho3D;

class PlatformData: public Component
{
    OBJECT(PlatformData);
public:
    PlatformData(Context* context);
    static void RegisterObject(Context* context);
    Vector2 p1;
    Vector2 p2;
    String type;
    Node* imagereference;
private:

};
