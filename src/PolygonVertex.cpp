#include "Urho3D/Scene/Node.h"
#include "PolygonVertex.h"
#include "Urho3D/Urho2D/RigidBody2D.h"
#include "Urho3D/Urho2D/CollisionCircle2D.h"
#include "Urho3D/Core/Context.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Urho2D/Sprite2D.h"
#include "Urho3D/Urho2D/StaticSprite2D.h"
#include "Urho3D/Math/Color.h"

PolygonVertex::PolygonVertex(Context* context): LogicComponent(context)
{
    SetUpdateEventMask(USE_UPDATE);
}

PolygonVertex::~PolygonVertex()
{

}

void PolygonVertex::RegisterObject(Context* context)
{
	context->RegisterFactory<PolygonVertex>();
}

void PolygonVertex::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Sprite2D* vertexsprite = cache->GetResource<Sprite2D>("Urho2D/vertex.png");
    if (!vertexsprite)
        return;
	StaticSprite2D* staticSprite = node_->CreateComponent<StaticSprite2D>();
	staticSprite->SetSprite(vertexsprite);
	staticSprite->SetLayer(60000);
	staticSprite->SetColor(Color(Color::WHITE,1));

    RigidBody2D* bodysprite = node_->CreateComponent<RigidBody2D>();
    bodysprite->SetBodyType(BT_STATIC);

    CollisionCircle2D* circle = node_->CreateComponent<CollisionCircle2D>();
    circle->SetRadius(0.1f);
}

void PolygonVertex::DelayedStart()
{
}

void PolygonVertex::Stop()
{
}

void PolygonVertex::Update(float timeStep)
{
}


void PolygonVertex::SetVector(Vector2 invector)
{
    vector_ = invector;
    node_->SetPosition2D(vector_);
}

Vector2 PolygonVertex::GetVector()
{
    return vector_;
}

void PolygonVertex::SetClear()
{
    isProcess = false;
    isSelected = false;
    isEvalue = false;
}

void PolygonVertex::setSelect()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::RED,1));
}

void PolygonVertex::setUnselect()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::WHITE,1));
}

void PolygonVertex::setSelectPolygon()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::YELLOW,1));
}
