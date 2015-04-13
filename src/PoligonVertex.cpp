#include "Node.h"
#include "PoligonVertex.h"
#include "RigidBody2D.h"
#include "CollisionCircle2D.h"
#include "Context.h"
#include "ResourceCache.h"
#include "Sprite2D.h"
#include "StaticSprite2D.h"
#include "Color.h"

PoligonVertex::PoligonVertex(Context* context): LogicComponent(context)
{
    SetUpdateEventMask(USE_UPDATE);
}

PoligonVertex::~PoligonVertex()
{

}

void PoligonVertex::RegisterObject(Context* context)
{
	context->RegisterFactory<PoligonVertex>();
}

void PoligonVertex::Start()
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

void PoligonVertex::DelayedStart()
{
}

void PoligonVertex::Stop()
{
}

void PoligonVertex::Update(float timeStep)
{
}


void PoligonVertex::SetVector(Vector2 invector)
{
    vector_ = invector;
    node_->SetPosition2D(vector_);
}

Vector2 PoligonVertex::GetVector()
{
    return vector_;
}

void PoligonVertex::SetClear()
{
    isProcess = false;
    isSelected = false;
    isEvalue = false;
}

void PoligonVertex::setSelect()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::RED,1));
}

void PoligonVertex::setUnselect()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::WHITE,1));
}

void PoligonVertex::setSelectPoligon()
{
    StaticSprite2D* staticSprite = GetComponent<StaticSprite2D>();
	staticSprite->SetColor(Color(Color::YELLOW,1));
}
