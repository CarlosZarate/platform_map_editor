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
#include "TmxFile2D.h"
#include "TileMap2D.h"
#include "TileMapLayer2D.h"

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
    Graphics* graphics = GetSubsystem<Graphics>();

    objprev_scene = new Scene(context_);
    objprev_scene->CreateComponent<Octree>();

    Graphics* graphidiscreetpositioncs = GetSubsystem<Graphics>();

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
    cameraNode_->SetPosition(Vector3(5.0f, 5.0f, 0.0f)); // Note that Z setting is discarded; use camera.zoom instead (see MoveCamera() below for example)

    camera_ = cameraNode_->CreateComponent<Camera>();
    camera_->SetOrthographic(true);

    graphics = GetSubsystem<Graphics>();
    camera_->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);

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

    nodeWall = scene_->CreateChild("NodoWall");

    SpriteSheet2D* SSTileSet = cache->GetResource<SpriteSheet2D>("Urho2D/tileset.xml");
    TileSetMap = SSTileSet->GetSpriteMapping();

    TmxFile2D* tmxFile = cache->GetResource<TmxFile2D>("Urho2D/map2.tmx");
    if (!tmxFile)
        return;

    SharedPtr<Node> tileMapNode(nodeWall->CreateChild("TileMap"));
    tileMapNode->SetPosition(Vector3(0.0f, 0.0f, 0.0f));

    TileMap2D* tileMap = tileMapNode->CreateComponent<TileMap2D>();
    // Set animation
    tileMap->SetTmxFile(tmxFile);

    // Set camera's position
    const TileMapInfo2D& info = tileMap->GetInfo();
    float x = info.GetMapWidth() * 0.5f;
    float y = info.GetMapHeight() * 0.5f;

    std::cout<<tileMap->GetNumLayers()<<std::endl;
    TileMapLayer2D* layer0 = tileMap->GetLayer(0);
    layer0->SetDrawOrder(1001);

    physicsWorld->DrawDebugGeometry();

    drawDebug_ = true; // Set DrawDebugGeometry() to true

    Sprite2D* topsprite = cache->GetResource<Sprite2D>("Urho2D/top.png");
    if (!topsprite)
        return;

    Sprite2D* floorsprite = cache->GetResource<Sprite2D>("Urho2D/floor.png");
    if (!floorsprite)
        return;

    /*floorNode = nodeWall->CreateChild("FloorSprite");
    floorNode->SetPosition(Vector3(7.5f, 7.5f, 0.0f));
    StaticSprite2D* floorstaticsprite = floorNode->CreateComponent<StaticSprite2D>();
    floorstaticsprite->SetSprite(floorsprite);
    floorstaticsprite->SetLayer(-500);

    topNode = nodeWall->CreateChild("TopSprite");
    topNode->SetPosition(Vector3(7.5f, 7.5f, 0.0f));
    taticSprite2D* topstaticsprite = topNode->CreateComponent<StaticSprite2D>();
    topstaticsprite->SetSprite(topsprite);
    topstaticsprite->SetLayer(1000);*/


    Sprite2D* object = cache->GetResource<Sprite2D>("Urho2D/object.png");
    if (!object)
        return;
    nodePlayer = scene_->CreateChild("NodoPlayer");
    StaticSprite2D* objectsprite = nodePlayer->CreateComponent<StaticSprite2D>();
	objectsprite->SetSprite(object);
	objectsprite->SetColor(Color::BLUE);
	objectsprite->SetLayer(1000);
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
    Vector2 point1, point2;

    if(dragPointEnd.x_ > dragPointBegin.x_)
    {
        if(dragPointEnd.y_ > dragPointBegin.y_)
        {
            point1 = Vector2(dragPointBegin);
            point2 = Vector2(dragPointEnd);
        }
        else
        {
            point1 = Vector2(dragPointBegin.x_, dragPointEnd.y_);
            point2 = Vector2(dragPointEnd.x_,dragPointBegin.y_);
        }
    }
    else
    {
        if(dragPointEnd.y_ > dragPointBegin.y_)
        {
            point1 = Vector2(dragPointEnd.x_, dragPointBegin.y_);
            point2 = Vector2(dragPointBegin.x_,dragPointEnd.y_);
        }
        else
        {
            point1 = Vector2(dragPointEnd);
            point2 = Vector2(dragPointBegin);
        }
    }

    for (float i = point1.x_; i <= point2.x_; i = i+0.5f)
    {
        for (float j = point1.y_; j <= point2.y_;j = j+0.5f)
        {
            if( j > 0 && i > 0 && !IntersectionBody(Vector2(i,j)))
            {
                Node* node  = nodeWall->CreateChild("RigidBody");
                node->SetPosition(Vector3(i, j, 0.0f));

                // Create rigid body
                RigidBody2D* body = node->CreateComponent<RigidBody2D>();
                body->SetBodyType(BT_STATIC);

                // Create box
                CollisionBox2D* box = node->CreateComponent<CollisionBox2D>();
                // Set size
                box->SetSize(Vector2(0.5f, 0.5f));
                // Set density
                box->SetDensity(1.0f);
                // Set friction
                box->SetFriction(0.5f);
                // Set restitution
                box->SetRestitution(0.1f);

                box->SetCategoryBits(32768);

                vectorShapes.Push(box);
            }
        }
    }
}

// Verifica colisiones verificando un punto con los fixtures en el mundo box2D
bool HelloGUI::IntersectionBody(Vector2 point)
{
    for (unsigned index = 0; index < vectorShapes.Size(); index++)
    {
        b2Fixture* testfixt = vectorShapes.At(index)->GetFixture();

        if ( testfixt->TestPoint( b2Vec2(point.x_, point.y_) ) )
        {
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
            vectorShapes.At(index)->GetNode()->Remove();
            vectorShapes.Erase(index);
            return true;
        }
    }

    return false;
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

        if ( position.x_ > RTop.x_ )
            RTop.x_ = position.x_;

        if ( position.x_ < LBotton.x_ )
            LBotton.x_ = position.x_;

        if ( position.y_ > RTop.y_ )
            RTop.y_ = position.y_;

        if ( position.y_ < LBotton.y_ )
            LBotton.y_ = position.y_;
    }
    return dimension;
}

void HelloGUI::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    // Punto inicial
    dragPointBegin = GetDiscreetPosition();
    // Punto final
    dragPointEnd = dragPointBegin;

    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    switch (currentFunction)
    {
        case DRAWWALL:
            DrawWall(eventData[P_BUTTON].GetInt());
            break;
        case DRAWENV:
            prevPositionLayer = GetDiscreetPosition();
            break;
        case DRAWCHAR:
            nodePlayer->SetPosition2D(GetDiscreetPosition());
            break;
    }

    SubscribeToEvent(E_MOUSEMOVE, HANDLER(HelloGUI, HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONUP, HANDLER(HelloGUI, HandleMouseButtonUp));
}


void HelloGUI::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    switch (currentFunction)
    {
        case DRAWWALL:
            dragPointEnd = GetDiscreetPosition();
            break;
        case DRAWENV:
            /*MoveLayerEnv(GetDiscreetPosition() - prevPositionLayer);
            prevPositionLayer = GetDiscreetPosition();*/
            break;
        case DRAWCHAR:
            nodePlayer->SetPosition2D(GetDiscreetPosition());
            break;
    }
}

void HelloGUI::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
    if (drawRectangle)
        CreateRectangleFixture();

    drawRectangle = false;

    UnsubscribeFromEvent(E_MOUSEMOVE);
    UnsubscribeFromEvent(E_MOUSEBUTTONUP);
}

void HelloGUI::DrawWall(int button)
{
    switch (button)
    {
        case 1:
            drawRectangle = true;
            break;
        case 4:
            DeletetFixtureWorld(dragPointBegin);
            break;
    }
}

void HelloGUI::DrawCharacter()
{
}

void HelloGUI::MoveLayerEnv(Vector2 envmov)
{
    if(currentEnv == TOP)
        topNode->Translate2D(envmov);
    if(currentEnv == FLOOR)
        floorNode->Translate2D(envmov);
}

Vector2 HelloGUI::GetMousePositionXY()
{
    Input* input = GetSubsystem<Input>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Vector3 screenPoint = Vector3((float)input->GetMousePosition().x_ / graphics->GetWidth(), (float)input->GetMousePosition().y_ / graphics->GetHeight(), 0.0f);

    Vector3 worldPoint = camera_->ScreenToWorldPoint(screenPoint);
    return Vector2(worldPoint.x_, worldPoint.y_);
}

Vector2 HelloGUI::GetDiscreetPosition()
{
    Vector2 discreetposition = GetMousePositionXY();
    discreetposition.x_ = (floor(discreetposition.x_/0.5f) * 0.5f) + 0.25f ;
    discreetposition.y_ = (floor(discreetposition.y_/0.5f) * 0.5f) + 0.25f ;

    return discreetposition;
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
    if(CurrentType == "Escenario")
    {
        ListView* ItemList = static_cast<ListView*>(eventData["Element"].GetPtr());
        Text* SelectedText = static_cast<Text*>(ItemList->GetSelectedItem());
        if(SelectedText->GetText() == "Top")
        {
            currentEnv = TOP;
        }
        if(SelectedText->GetText() == "Floor")
        {
            currentEnv = FLOOR;
        }
    }
    if(CurrentType == "Characters")
    {
        ListView* ItemList = static_cast<ListView*>(eventData["Element"].GetPtr());
        Text* SelectedText = static_cast<Text*>(ItemList->GetSelectedItem());
        if(SelectedText->GetText() == "Player")
        {
            currentCharType = PLAYER;
        }
        if(SelectedText->GetText() == "Enemy")
        {
            currentCharType = ENEMY;
        }
        if(SelectedText->GetText() == "NPC")
        {
            currentCharType = NPC;
        }
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
    if(type == "Wall")
        currentFunction = DRAWWALL;
    if(type == "Characters")
        currentFunction = DRAWCHAR;
    if(type == "Escenario")
        currentFunction = DRAWENV;
}

void HelloGUI::CreateGrids()
{
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

    /// Lineas verticales
    for (float i = 0; i <= 50; i+=0.5f)
    {
        debug->AddLine( Vector3(i, 0, 0),
                        Vector3(i, 50, 0),
                        Color(0, 1, 1, 1),
                        false );
    }

    /// Lineas horizontales
    for (float j = 0; j <= 50; j+=0.5)
    {
        debug->AddLine( Vector3(0, j, 0),
                        Vector3(50, j, 0),
                        Color(0, 1, 1, 1),
                        false );
    }

    debug->AddLine(Vector3(-3, 0, 0), Vector3(50, 0, 0), Color(1, 0, 0, 0),  false);
    debug->AddLine(Vector3(0, 50, 0), Vector3(0, -3, 0), Color(0, 0, 1, 0),  false);

}

void HelloGUI::MoveCamera(float timeStep)
{

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

    if (input->GetKeyPress(KEY_F5))
    {
        File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/nodo_map.xml", FILE_WRITE);
        nodeWall->SaveXML(saveFile);

        JSONFile* data = new JSONFile(context_);
        JSONValue MapNodeJson = data->CreateRoot();
        JSONValue blockarray = MapNodeJson.CreateChild("blocks",JSON_ARRAY);

        for (unsigned index = 0; index < vectorShapes.Size(); index++)
        {
            Vector2 prevector = vectorShapes.At(index)->GetNode()->GetPosition2D()*2;
            IntVector2 vector2(prevector.x_,prevector.y_);
            blockarray.AddIntVector2(vector2);
        }

        MapNodeJson.SetVector2("playerpost",nodePlayer->GetPosition2D());
        JSONValue characters = MapNodeJson.CreateChild("characters",JSON_ARRAY);
        JSONValue character = characters.CreateChild(JSON_OBJECT);
        character.SetString("name","Player");
        character.SetString("source","April.scml");
        character.SetBool("anim",true);
        character.SetFloat("radio",0.16f);

        File file(context_,GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/MapNode.json", FILE_WRITE);
        data->Save(file);
    }
    if (input->GetKeyPress(KEY_F7))
    {
        nodeWall->RemoveAllChildren();
        nodeWall->RemoveAllComponents();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile* nodoXMLFile = cache->GetResource<XMLFile>("Scenes/nodo_map.xml");
        XMLElement nodoXML(nodoXMLFile->GetRoot());
        nodeWall->LoadXML(nodoXML);
        std::cout<<"Nodo cargado"<<std::endl;
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





