
#pragma once

#include "Vector2.h"
#include "LogicComponent.h"

using namespace Urho3D;

class PoligonVertex: public LogicComponent
{
    OBJECT(PoligonVertex);
public:
    PoligonVertex(Context* context);
    ~PoligonVertex();
    static void RegisterObject(Context* context);
    /// Called when the component is added to a scene node. Other components may not yet exist.
    virtual void Start();
    /// Called before the first update. At this point all other components of the node should exist. Will also be called if update events are not wanted; in that case the event is immediately unsubscribed afterward.
    virtual void DelayedStart();
    /// Called when the component is detached from a scene node, usually on destruction.
    virtual void Stop();
    /// Called on scene update, variable timestep.
    virtual void Update(float timeStep);

    void SetVector(Vector2 invector);
    Vector2 GetVector();
    bool isProcess = false;
    bool isSelected = false;
    bool isEvalue = false;

    void SetClear();
    void setSelect();
    void setUnselect();
    void setSelectPoligon();
private:
    Vector2 vector_;
};
