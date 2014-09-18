//
// Copyright (c) 2008-2014 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include <iostream>

#include "Button.h"
#include "BorderImage.h"
#include "CheckBox.h"
#include "CoreEvents.h"
#include "AnimatedSprite2D.h"
#include "AnimationSet2D.h"
#include "SpriteSheet2D.h"
#include "Camera.h"
#include "Octree.h"
#include "Engine.h"
#include "Font.h"
#include "Graphics.h"
#include "Input.h"
#include "LineEdit.h"
#include "Renderer.h"
#include "ResourceCache.h"
#include "Scene.h"
#include "Sprite2D.h"
#include "StaticSprite2D.h"
#include "DebugRenderer.h"
#include "PhysicsWorld2D.h"
#include "Text.h"
#include "Zone.h"
#include "Texture2D.h"
#include "ToolTip.h"
#include "UI.h"
#include "UIElement.h"
#include "UIEvents.h"
#include "View3D.h"
#include "Window.h"
#include "DropDownList.h"
#include "ListView.h"
#include "JSONFile.h"
#include "JSONValue.h"
#include "File.h"
#include "Deserializer.h"

#include "Script.h"
#include "ScriptFile.h"
#include "ScriptInstance.h"

#include "HelloGUI.h"

#include "DebugNew.h"

// Librerias Box2D
#include "CollisionBox2D.h"
#include "CollisionCircle2D.h"
#include "CollisionEdge2D.h"
#include "CollisionPolygon2D.h"
#include "ConstraintDistance2D.h"
#include "ConstraintFriction2D.h"
#include "ConstraintGear2D.h"
#include "ConstraintMotor2D.h"
#include "ConstraintMouse2D.h"
#include "ConstraintPrismatic2D.h"
#include "ConstraintPulley2D.h"
#include "ConstraintRevolute2D.h"
#include "ConstraintRope2D.h"
#include "ConstraintWeld2D.h"
#include "ConstraintWheel2D.h"
#include "RigidBody2D.h"

// Number of static sprites to draw
static const unsigned NUM_SPRITES = 200;
static const StringHash VAR_MOVESPEED("MoveSpeed");
static const StringHash VAR_ROTATESPEED("RotateSpeed");

// Detect node screen
//Node* pickedNode;
//RigidBody2D* dummyBody;

DEFINE_APPLICATION_MAIN(HelloGUI)

HelloGUI::HelloGUI(Context* context) :
    Sample(context),
    uiRoot_(GetSubsystem<UI>()->GetRoot()),
    dragBeginPosition_(IntVector2::ZERO)
{
}

void HelloGUI::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    // Setup the viewport for displaying the scene
    SetupViewport();

    // Enable OS cursor
    GetSubsystem<Input>()->SetMouseVisible(true);

    // Load XML file containing default UI style sheet
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");

    // Set the loaded style as default style
    uiRoot_->SetDefaultStyle(style);

    // Initialize Window
    InitWindow();

    // Hook up to the frame update events
    SubscribeToEvents();
}

void HelloGUI::CreateScene()
{
    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();
    PhysicsWorld2D* physicsWorld = scene_->CreateComponent<PhysicsWorld2D>();

    objprev_scene = new Scene(context_);
    objprev_scene->CreateComponent<Octree>();

    Graphics* graphics = GetSubsystem<Graphics>();

    ObjPrevCameraNode_ = objprev_scene->CreateChild("Camera");
    // Set camera's position
    ObjPrevCameraNode_->SetPosition(Vector3(0.0f, 0.0f, -10.0f));

    Camera* objprevcamera = ObjPrevCameraNode_->CreateComponent<Camera>();
    objprevcamera->SetOrthographic(true);

    objprevcamera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);
    objprevcamera->SetZoom(objprevcamera->GetZoom() * 10.0f);

    /// Create camera node
    cameraNode_ = scene_->CreateChild("Camera");
    // Set camera's position
    //cameraNode_->SetPosition(Vector3(0.0f, 0.0f, -10.0f));
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetOrthographic(true);
    camera->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);

    /// Camara del editor
    // Create camera
    cameraNode_ = scene_->CreateChild("Camera");
    // Set camera's position
    cameraNode_->SetPosition(Vector3(0.0f, 0.0f, 0.0f)); // Note that Z setting is discarded; use camera.zoom instead (see MoveCamera() below for example)

    camera_ = cameraNode_->CreateComponent<Camera>();
    camera_->SetOrthographic(true);

    graphics = GetSubsystem<Graphics>();
    camera_->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);
    ///

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Sprite2D* bgsprite = cache->GetResource<Sprite2D>("Urho2D/backgroung.png");
    if (!bgsprite)
        return;
    SharedPtr<Node> bgspriteNode(objprev_scene->CreateChild("StaticSprite2D"));
    StaticSprite2D* bgstaticSprite = bgspriteNode->CreateComponent<StaticSprite2D>();
    bgstaticSprite->SetSprite(bgsprite);
    bgstaticSprite->SetLayer(1);

    // Get animation set
    AnimationSet2D* animationSet = cache->GetResource<AnimationSet2D>("Urho2D/April.scml");
    if (!animationSet)
        return;

    SharedPtr<Node> PreviewNode(objprev_scene->CreateChild("PrevNode"));
    PreviewNode->SetPosition(Vector3(0.0f, -0.2f, -1.0f));

    AnimatedSprite2D* animatedSprite = PreviewNode->CreateComponent<AnimatedSprite2D>();
    // Set animation
    animatedSprite->SetAnimation(animationSet, "Run");
    animatedSprite->SetLayer(2);

    SpriteSheet2D* SSTileSet = cache->GetResource<SpriteSheet2D>("Urho2D/tileset.xml");
    TileSetMap = SSTileSet->GetSpriteMapping();

    physicsWorld->DrawDebugGeometry();

    drawDebug_ = true; // Set DrawDebugGeometry() to true

    // Incializando nodo y body del mapa
    mapNode = scene_->CreateChild("mapNode");
    mapRigidBody = mapNode->CreateComponent<RigidBody2D>();
    mapRigidBody->SetBodyType(BT_STATIC);
    mapRigidBody->SetLinearDamping(0.0f);
    mapRigidBody->SetAngularDamping(0.0f);
}

// Dibuja los bordes de un  un rectangulo
void HelloGUI::DrawRectangle(Rect rect)
{
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

    Vector3 point1(rect.min_, 0);
    Vector3 point3(rect.max_, 0);
    Vector3 point2(point1.x_, point3.y_, 0);
    Vector3 point4(point3.x_, point1.y_, 0);
    Color color(1, 0, 0, 1);

    debug->AddLine( point1, point2, color, false );
    debug->AddLine( point2, point3, color, false );
    debug->AddLine( point3, point4, color, false );
    debug->AddLine( point4, point1, color, false );
}

// Crea un rentangule CollisionShape (fixture)
void HelloGUI::CreateRectangleFixture()
{
    Node* mapNode = scene_->GetChild("mapNode", true);
    CollisionPolygon2D* rectangleShape = mapNode->CreateComponent<CollisionPolygon2D>();

    if(dragPointEnd.x_ > dragPointBegin.x_)
    {
        if(dragPointEnd.y_ > dragPointBegin.y_)
        {
            dragPointEnd.x_ += 0.25f;
            dragPointEnd.y_ += 0.25f;
            dragPointBegin.x_ -= 0.25f;
            dragPointBegin.y_ -= 0.25f;
        }
        else
        {
            dragPointEnd.x_ += 0.25f;
            dragPointEnd.y_ -= 0.25f;
            dragPointBegin.x_ -= 0.25f;
            dragPointBegin.y_ += 0.25f;
        }
    }
    else
    {
        if(dragPointEnd.y_ > dragPointBegin.y_)
        {
            dragPointEnd.x_ -= 0.25f;
            dragPointEnd.y_ += 0.25f;
            dragPointBegin.x_ += 0.25f;
            dragPointBegin.y_ -= 0.25f;
        }
        else
        {
            dragPointEnd.x_ -= 0.25f;
            dragPointEnd.y_ -= 0.25f;
            dragPointBegin.x_ += 0.25f;
            dragPointBegin.y_ += 0.25f;
        }
    }

    Vector2 point1(dragPointBegin);
    Vector2 point3(dragPointEnd);
    Vector2 point2(point3.x_, point1.y_);
    Vector2 point4(point1.x_, point3.y_);

    PODVector<Vector2> vertices;
    vertices.Push(point1);
    vertices.Push(point2);
    vertices.Push(point3);
    vertices.Push(point4);

    rectangleShape->SetVertices(vertices);
    mapRigidBody->  AddCollisionShape2D(rectangleShape);
    vectorShapes.Push(rectangleShape);
    std::cout<<vectorShapes.Size()<<std::endl;
}

// Verifica colisiones verificando un punto con los fixtures en el mundo box2D
bool HelloGUI::IntersectionBody(Vector2 point)
{
    for (unsigned index = 0; index < vectorShapes.Size(); index++)
    {
        b2Fixture* testfixt = vectorShapes.At(index)->GetFixture();

        if ( testfixt->TestPoint( b2Vec2(point.x_, point.y_) ) )
        {
            std::cout << "Existe fixture" << std::endl;
            return true;
        }
    }

    return false;
}

// Verifica y eliimina el CollsionSahpe dado en un punto
bool HelloGUI::DeletetFixtureWorld(Vector2 point)
{
    for (unsigned index = 0; index < vectorShapes.Size(); index++)
    {
        b2Fixture* fixture = vectorShapes.At(index)->GetFixture();

        if ( fixture->TestPoint( b2Vec2(point.x_, point.y_) ) )
        {
            mapNode->RemoveComponent(vectorShapes.At(index));
            vectorShapes.Erase(index);
            return true;
        }
    }

    return false;
}


void HelloGUI::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    dragPointEnd    = GetMousePositionXY();
    dragPointEnd.x_ = (floor(dragPointEnd.x_/0.5f) * 0.5f) + 0.25f;
    dragPointEnd.y_ = (floor(dragPointEnd.y_/0.5f) * 0.5f) + 0.25f;

    Vector2 point1(dragPointBegin);
    Vector2 point3(dragPointEnd);
    Vector2 point2(point3.x_, point1.y_);
    Vector2 point4(point1.x_, point3.y_);

    if (IntersectionBody(point1))
        drawRectangle = false;

    if (IntersectionBody(point2))
        drawRectangle = false;

    if (IntersectionBody(point3))
        drawRectangle = false;

    if (IntersectionBody(point4))
        drawRectangle = false;
}

// Crea un nodo con con todos sus componentes (textua y body) del tamaño 32px
void HelloGUI::CreateNode(Vector3 position)
{
    SharedPtr<Node> box(scene_->CreateChild("Box"));
    box->SetPosition(position);

    RigidBody2D* boxBody = box->CreateComponent<RigidBody2D>();
    boxBody->SetBodyType(BT_STATIC);
    boxBody->SetLinearDamping(0.0f);
    boxBody->SetAngularDamping(0.0f);

    CollisionBox2D* shape = box->CreateComponent<CollisionBox2D>(); // Create box shape

    shape->SetSize(Vector2(0.5, 0.5)); // Set size
    shape->SetDensity(1.0f); // Set shape density (kilograms per meter squared)
    shape->SetFriction(0.5f); // Set friction
    shape->SetRestitution(0.1f); // Set restitution (slight bounce)

    vectorNodes_.Push(box);
    std::cout<<vectorNodes_.Size()<<std::endl;
}

// Calcula los limites que ocupa los body y los fixtures creados en el mapa
Rect HelloGUI::GetMatrixLength()
{
    Rect dimension(0, 0, 0, 0);

    Vector2 LBotton(0, 0);
    Vector2 RTop(0, 0);

    for (unsigned i = 0; i < vectorNodes_.Size(); i++)
    {
        SharedPtr<Node> node = vectorNodes_[i];

        Vector3 position = node->GetPosition();
        std::cout << "Node " << i << ": " << position.x_ << ", " << position.y_ << std::endl;

        if ( position.x_ > RTop.x_ )
            RTop.x_ = position.x_;

        if ( position.x_ < LBotton.x_ )
            LBotton.x_ = position.x_;

        if ( position.y_ > RTop.y_ )
            RTop.y_ = position.y_;

        if ( position.y_ < LBotton.y_ )
            LBotton.y_ = position.y_;
    }
    std::cout << "--------------------------------" << std::endl;

    dimension = Rect(LBotton.x_, RTop.y_,
                        RTop.x_, LBotton.y_);

    std::cout << "IntRect: (" << LBotton.x_ << "," << RTop.y_ << ") , "
                              << RTop.x_ << "," << LBotton.y_ << std::endl;
    return dimension;
}

void HelloGUI::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();

    // Punto inicial
    dragPointBegin = GetMousePositionXY();
    dragPointBegin.x_ = (floor(dragPointBegin.x_/0.5f) * 0.5f) + 0.25f ;
    dragPointBegin.y_ = (floor(dragPointBegin.y_/0.5f) * 0.5f) + 0.25f ;

    // Punto final
    dragPointEnd = GetMousePositionXY();
    dragPointEnd.x_ = (floor(dragPointEnd.x_/0.5f) * 0.5f) + 0.25f;
    dragPointEnd.y_ = (floor(dragPointEnd.y_/0.5f) * 0.5f) + 0.25f ;

    if (input->GetMouseButtonDown(1))
    {
        std::cout << "Button 1 Down" << std::endl;
        drawRectangle = true;
    }

    if (input->GetMouseButtonDown(4))
    {
        // Elimando fixture crear al hacer clic nuevamente
        Vector2 fixturePoint(dragPointBegin);

        if ( DeletetFixtureWorld(fixturePoint) )
            std::cout << "Se ha eliminado el fixture!" << std::endl;


        std::cout << "Button 4 Down" << std::endl;
    }

    SubscribeToEvent(E_MOUSEMOVE, HANDLER(HelloGUI, HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONUP, HANDLER(HelloGUI, HandleMouseButtonUp));
}

void HelloGUI::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
    std::cout << "Mouse button 3 UP" << std::endl;

    if (drawRectangle)
        CreateRectangleFixture();

    drawRectangle = false;

    UnsubscribeFromEvent(E_MOUSEMOVE);
    UnsubscribeFromEvent(E_MOUSEBUTTONUP);
}

Vector2 HelloGUI::GetMousePositionXY()
{
    Input* input = GetSubsystem<Input>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Vector3 screenPoint = Vector3((float)input->GetMousePosition().x_ / graphics->GetWidth(), (float)input->GetMousePosition().y_ / graphics->GetHeight(), 0.0f);

    Vector3 worldPoint = camera_->ScreenToWorldPoint(screenPoint);
    return Vector2(worldPoint.x_, worldPoint.y_);
}

void HelloGUI::SetupViewport()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    Renderer* renderer = GetSubsystem<Renderer>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    renderer->SetNumViewports(1);

    // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
    //renderer->GetDefaultZone()->SetFogColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
}

void HelloGUI::InitWindow()
{
    window_ = new Window(context_);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UI* ui = GetSubsystem<UI>();

    SharedPtr<UIElement> auxwindow;
    auxwindow = ui->LoadLayout(cache->GetResource<XMLFile>("UI/map_editor.xml"));
    auxwindow->SetMinHeight(440);
    auxwindow->SetPosition(1130,10);

    uiRoot_->AddChild(auxwindow);

    window_ = static_cast<Window*>(uiRoot_->GetChild("EditorMenu",true));

    View3D* auxview = (View3D*)auxwindow->GetChild("ObjPrevView",true);
    //Scene* newscene = auxview->GetScene();
    auxview->SetView(objprev_scene,ObjPrevCameraNode_->GetComponent<Camera>());

    ListView* itemlist = (ListView*)auxwindow->GetChild("FileList",true);

    JSONFile* data = new JSONFile(context_);
    File file(context_,"Data/Urho2D/map_editor.json");
    data->Load(file);

    rootjson = data->GetRoot();

    DropDownList* DropDownType = (DropDownList*)auxwindow->GetChild("ObjeList",true);
    Text* SelectedText = static_cast<Text*>(DropDownType->GetSelectedItem());
    LoadSelectedType(SelectedText->GetText());

    SubscribeToEvent(DropDownType, E_ITEMSELECTED, HANDLER(HelloGUI, HandleChangeType));
    SubscribeToEvent(itemlist, E_ITEMSELECTED, HANDLER(HelloGUI, HandleLoadPreview));
    SubscribeToEvent(ui->GetRoot(), E_HOVERBEGIN, HANDLER(HelloGUI, HandleInWindow));
    SubscribeToEvent(ui->GetRoot(), E_HOVEREND, HANDLER(HelloGUI, HandleOutWindow));
}

void HelloGUI::HandleInWindow(StringHash eventType, VariantMap& eventData)
{
    std::cout<<"Entre"<<std::endl;
}
void HelloGUI::HandleOutWindow(StringHash eventType, VariantMap& eventData)
{
    std::cout<<"Salir"<<std::endl;
}

void HelloGUI::HandleChangeType(StringHash eventType, VariantMap& eventData)
{
    DropDownList* DropDownType = static_cast<DropDownList*>(eventData["Element"].GetPtr());
    Text* SelectedText = static_cast<Text*>(DropDownType->GetSelectedItem());
    LoadSelectedType(SelectedText->GetText());
}

void HelloGUI::HandleLoadPreview(StringHash eventType, VariantMap& eventData)
{
    if(CurrentType == "Tile")
    {
        ListView* ItemList = static_cast<ListView*>(eventData["Element"].GetPtr());
        Text* SelectedText = static_cast<Text*>(ItemList->GetSelectedItem());
        Sprite2D* currenttile = TileSetMap[SelectedText->GetText()];

        objprev_scene->GetChild("PrevNode",true)->RemoveAllComponents();
        objprev_scene->GetChild("PrevNode",true)->Remove();
        SharedPtr<Node> PreviewNode(objprev_scene->CreateChild("PrevNode"));
        StaticSprite2D* staticSprite = PreviewNode->CreateComponent<StaticSprite2D>();
        staticSprite->SetSprite(currenttile);
        staticSprite->SetLayer(2);
    }
}

void HelloGUI::LoadSelectedType(String type)
{
    CurrentType = type;
    ListView* itemlist = (ListView*)window_->GetChild("FileList",true);
    itemlist->RemoveAllItems();

    if(type == "Tile")
    {
        Vector<String> keys = TileSetMap.Keys();
        for( int i = 0 ; i < keys.Size() ; i++ )
        {
            Text* item = new Text(context_);
            item->SetText(keys[i]);
            item->SetStyle("FileSelectorListText");
            itemlist->InsertItem(itemlist->GetNumItems(), item);
        }
    }
    else
    {
        JSONValue jsontype = rootjson.GetChild(type);
        for(int i = 0 ; i < jsontype.GetSize() ; i++)
        {
            Text* item = new Text(context_);
            item->SetText(jsontype.GetString(i));
            item->SetStyle("FileSelectorListText");
            itemlist->InsertItem(itemlist->GetNumItems(), item);
        }
    }

}

void HelloGUI::HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    Text* windowTitle = static_cast<Text*>(window_->GetChild("WindowTitle", true));

    // Get control that was clicked
    UIElement* clicked = static_cast<UIElement*>(eventData[UIMouseClick::P_ELEMENT].GetPtr());

    String name = "...?";
    if (clicked)
    {
        // Get the name of the control that was clicked
        name = clicked->GetName();
    }

    // Update the Window's title text
    windowTitle->SetText("Hello " + name + "!");
}

void HelloGUI::CreateGrids()
{
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

    /// Lineas verticales
    for (float i = -12; i <= 30; i+=0.5f)
    {
        debug->AddLine( Vector3(i, -10, 0),
                        Vector3(i, 10, 0),
                        Color(0, 1, 1, 1),
                        false );
    }

    /// Lineas horizontales
    for (float j = -10; j <= 10; j+=0.5)
    {
        debug->AddLine( Vector3(-12, j, 0),
                        Vector3(30, j, 0),
                        Color(0, 1, 1, 1),
                        false );
    }

    debug->AddLine(Vector3(-12, 0, 0), Vector3(12, 0, 0), Color(1, 0, 0, 0),  false);
    debug->AddLine(Vector3(0, 12, 0), Vector3(0, -12, 0), Color(0, 0, 1, 0),  false);

}

void HelloGUI::MoveCamera(float timeStep)
{
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 4.0f;

    // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
    if (input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3::UP * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3::DOWN * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
    if (input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

    if (input->GetKeyDown(KEY_PAGEUP))
    {
        Camera* camera = cameraNode_->GetComponent<Camera>();
        camera->SetZoom(camera->GetZoom() * 1.01f);
    }

    if (input->GetKeyDown(KEY_PAGEDOWN))
    {
        Camera* camera = cameraNode_->GetComponent<Camera>();
        camera->SetZoom(camera->GetZoom() * 0.99f);
    }
}

void HelloGUI::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, HANDLER(HelloGUI, HandleUpdate));


    // Subscribe to mouse click
    SubscribeToEvent(E_MOUSEBUTTONDOWN, HANDLER(HelloGUI, HandleMouseButtonDown));
}

void HelloGUI::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
    Input* input = GetSubsystem<Input>();

    // Toggle physics debug geometry with space
    if (input->GetKeyPress(KEY_SPACE))
        drawDebug_ = !drawDebug_;

    if (drawDebug_)
        physicsWorld->DrawDebugGeometry();

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);

    // Dibuja las griilas del mapa
    this->CreateGrids();

    // Dibuja un rectangulo si el mouse es arrastrado en pantalla.
    if (drawRectangle)
        this->DrawRectangle( Rect(dragPointBegin, dragPointEnd) );


    Graphics* graphics = GetSubsystem<Graphics>();
    float halfWidth = (float)graphics->GetWidth() * 0.5f * PIXEL_SIZE;
    float halfHeight = (float)graphics->GetHeight() * 0.5f * PIXEL_SIZE;
}

void HelloGUI::HandleDragMoveViewport(StringHash eventType, VariantMap& eventData)
{
   UIElement* draggedElement = static_cast<UIElement*>(eventData["Element"].GetPtr()); // Get the dragged UI element (camWindow)
   int posX=draggedElement->GetPosition().x_; // Get current Window left position
   int posY=draggedElement->GetPosition().y_; // Get current Window top position
   GetSubsystem<Renderer>()->GetViewport(1)->SetRect(IntRect(posX, posY, posX + draggedElement->GetWidth() , posY + draggedElement->GetHeight()));
}





