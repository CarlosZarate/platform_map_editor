#include "Urho3D/UI/Button.h"
#include "Urho3D/UI/CheckBox.h"
#include "Urho3D/Core/CoreEvents.h"
#include "Urho3D/Urho2D/AnimatedSprite2D.h"
#include "Urho3D/Urho2D/AnimationSet2D.h"
#include "Urho3D/Urho2D/SpriteSheet2D.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Graphics/Octree.h"
#include "Urho3D/Engine/Engine.h"
#include "Urho3D/UI/Font.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/UI/LineEdit.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Urho2D/Sprite2D.h"
#include "Urho3D/Graphics/DebugRenderer.h"
#include "Urho3D/Urho2D/PhysicsWorld2D.h"
#include "Urho3D/UI/Text.h"
#include "Urho3D/Graphics/Zone.h"
#include "Urho3D/Graphics/Texture2D.h"
#include "Urho3D/UI/ToolTip.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/UI/UIEvents.h"
#include "Urho3D/UI/View3D.h"
#include "Urho3D/UI/DropDownList.h"
#include "Urho3D/UI/ListView.h"
#include "Urho3D/Resource/JSONFile.h"
#include "Urho3D/Urho2D/TmxFile2D.h"
#include "Urho3D/Urho2D/TileMap2D.h"
#include "Urho3D/Urho2D/TileMapLayer2D.h"

#include "Urho3D/AngelScript/Script.h"
#include "Urho3D/AngelScript/ScriptFile.h"
#include "Urho3D/AngelScript/ScriptInstance.h"


#include "PlatformData.h"
#include "ObjectData.h"

// Librerias Box2D
#include "Urho3D/Urho2D/CollisionBox2D.h"
#include "Urho3D/Urho2D/CollisionCircle2D.h"
#include "Urho3D/Urho2D/CollisionEdge2D.h"
#include "Urho3D/Urho2D/CollisionPolygon2D.h"
#include "Urho3D/Urho2D/RigidBody2D.h"

#include "MapEditor.h"

URHO3D_DEFINE_APPLICATION_MAIN(MapEditor)

MapEditor::MapEditor(Context* context) :
    Sample(context),
    uiRoot_(GetSubsystem<UI>()->GetRoot()),
    dragBeginPosition_(IntVector2::ZERO)
{
	PolygonVertex::RegisterObject(context);
	PlatformData::RegisterObject(context);
	ObjectData::RegisterObject(context);
}

void MapEditor::Start()
{
    // Execute base class startup
    Sample::Start();

    // Create the scene content
    CreateScene();

    CreatePreviewScene();

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

void MapEditor::CreatePreviewScene()
{
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

    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Sprite2D* bgsprite = cache->GetResource<Sprite2D>("Urho2D/backgroung.png");
    if (!bgsprite)
        return;
    SharedPtr<Node> bgspriteNode(objprev_scene->CreateChild("StaticSprite2D"));
    StaticSprite2D* bgstaticSprite = bgspriteNode->CreateComponent<StaticSprite2D>();
    bgstaticSprite->SetSprite(bgsprite);
    bgstaticSprite->SetLayer(1);

    // Get animation set
    AnimationSet2D* animationSet = cache->GetResource<AnimationSet2D>("Urho2D/gladiador.scml");
    if (!animationSet)
        return;

    SharedPtr<Node> PreviewNode(objprev_scene->CreateChild("PrevNode"));
    PreviewNode->SetPosition(Vector3(0.0f, -0.2f, -1.0f));

    AnimatedSprite2D* animatedSprite = PreviewNode->CreateComponent<AnimatedSprite2D>();
    // Set animation
    animatedSprite->SetAnimationSet(animationSet);
    animatedSprite->SetAnimation("run");
    animatedSprite->SetLayer(2);
    PreviewNode->SetScale(0.1f);
}

void MapEditor::CreateScene()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);
    scene_->CreateComponent<Octree>();
    scene_->CreateComponent<DebugRenderer>();
    PhysicsWorld2D* physicsWorld = scene_->CreateComponent<PhysicsWorld2D>();

    cameraNode_ = scene_->CreateChild("Camera");
    cameraNode_->SetPosition2D(Vector2(5.0f, 5.0f));
    camera_ = cameraNode_->CreateComponent<Camera>();
    camera_->SetOrthographic(true);
    camera_->SetOrthoSize((float)graphics->GetHeight() * PIXEL_SIZE);


    nodeWall = scene_->CreateChild("NodoWall");

    SpriteSheet2D* SSTileSet = cache->GetResource<SpriteSheet2D>("Urho2D/tileset.xml");
    TileSetMap = SSTileSet->GetSpriteMapping();

    TmxFile2D* tmxFile = cache->GetResource<TmxFile2D>("Urho2D/nivel1.tmx");
    if (!tmxFile)
        return;

    SharedPtr<Node> tileMapNode(scene_->CreateChild("TileMap"));
    tileMap = tileMapNode->CreateComponent<TileMap2D>();
    tileMap->SetTmxFile(tmxFile);

    drawDebug_ = true;

    Sprite2D* object = cache->GetResource<Sprite2D>("Urho2D/object.png");
    if (!object)
        return;
    nodePlayer = scene_->CreateChild("NodoPlayer");
    StaticSprite2D* objectsprite = nodePlayer->CreateComponent<StaticSprite2D>();
	objectsprite->SetSprite(object);
	objectsprite->SetColor(Color::BLUE);
	objectsprite->SetLayer(1000);
}

void MapEditor::SetupViewport()
{
    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->SetNumViewports(1);
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);
}

void MapEditor::CreateGrids()
{
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();
    /// Lineas verticales
    for (float i = 0; i <= 100; i+=0.7f)
    {
        debug->AddLine( Vector3(i, 0, 0),
                        Vector3(i, 100, 0),
                        Color(0, 1, 1, 0.5f),
                        false );
    }
    /// Lineas horizontales
    for (float j = 0; j <= 100; j+=0.7)
    {
        debug->AddLine( Vector3(0, j, 0),
                        Vector3(100, j, 0),
                        Color(0, 1, 1, 0.5f),
                        false );
    }
}

void MapEditor::LoadMap()
{
    nodeWall->RemoveAllChildren();

    JSONFile* data = new JSONFile(context_);
    File datafile(context_, "Data/Scenes/MapNode.json");
    data->Load(datafile);
    JSONValue rootjson = data->GetRoot();
    JSONArray platforms = rootjson.Get("platforms").GetArray();

    PlatformsList.Clear();
    for(int i = 0 ; i < platforms.Size() ; i++)
    {
        JSONValue platformdata = platforms[i];
        String type = platformdata.Get("type").GetString();
        Vector2 p1(platformdata.Get("p1_x").GetFloat(),platformdata.Get("p1_y").GetFloat());
        Vector2 p2(platformdata.Get("p2_x").GetFloat(),platformdata.Get("p2_y").GetFloat());
        if(type == "movplatform")
        {
            CreateMovablePlatform(p1, p2);
        }
        else
        {
            CreatePlatform(p1, p2,type);
        }
    }

    ObjectList.Clear();
    JSONArray objects = rootjson.Get("objects").GetArray();
    for(int i = 0 ; i < objects.Size() ; i++)
    {
        JSONValue object = objects[i];
        String type = object.Get("type").GetString();
        if(type == "enemy")
        {
            Vector2 pos(object.Get("pos_x").GetFloat(),object.Get("pos_y").GetFloat());
            CreateEnemy(pos);
        }
    }

    Vector2 posPlayer(rootjson.Get("playerPos_x").GetFloat(),rootjson.Get("playerPos_y").GetFloat());
    nodePlayer->SetPosition2D(posPlayer);

    JSONFile* mapData = new JSONFile(context_);
    File mapDatafile(context_, "Data/Scenes/MapData.json");
    mapData->Load(mapDatafile);
    JSONValue rootDataJson = mapData->GetRoot();
    JSONArray polygonsJSON = rootDataJson.Get("polygons").GetArray();

    Vector<String> keys = PolygonMap.Keys();
    for(int i = 0; i < keys.Size(); i++)
    {
        RemovePolygon(keys[i]);
    }
    while(!ListNodePolygonsPhysics.Empty())
    {
        Node* nodeRemove = ListNodePolygonsPhysics.Back();
        if(nodeRemove)
        {
            nodeRemove->Remove();
            ListNodePolygonsPhysics.Pop();
        }
    }
    PolygonMap.Clear();
    ListPolygonTriangle.Clear();
    PolygonCounter = 0;
    for(int i = 0 ; i < polygonsJSON.Size() ; i++)
    {
        JSONArray polygonVertexArray = polygonsJSON[i].GetArray();
        Vector<PolygonVertex *>* polygon_ = new Vector<PolygonVertex *>();
        PolygonMap.Insert(Pair<String, Vector<PolygonVertex *>*>("Polygon" + String(PolygonCounter), polygon_));
        PolygonCounter++;
        for(int j = 0; j < polygonVertexArray.Size(); j++)
        {
            Vector2 v(polygonVertexArray[j].Get("x_").GetFloat(), polygonVertexArray[j].Get("y_").GetFloat());
            polygon_->Push(CreatePolygonVertex(v));
        }
        UnselectPolygon(CurrentPolygon);
        CurrentPolygon = polygon_;
        SelectPolygon(polygon_);
    }
    LoadPolygonList();
}

void MapEditor::SaveMap()
{
    JSONFile* data = new JSONFile(context_);
    JSONValue* MapNodeJson = &data->GetRoot();
    Vector2 playerPos = nodePlayer->GetPosition2D();
    MapNodeJson->Set("playerPos_x", JSONValue(playerPos.x_));
    MapNodeJson->Set("playerPos_y", JSONValue(playerPos.y_));
    /*JSONValue characters = MapNodeJson.CreateChild("characters",JSON_ARRAY);
    JSONValue character = characters.CreateChild(JSON_OBJECT);
    character.SetString("name","Player");
    character.SetString("source","April.scml");
    character.SetBool("anim",true);
    character.SetFloat("radio",0.16f);*/

    JSONArray triangleArray;// = MapNodeJson.CreateChild("triangles",JSON_ARRAY);

    for(RandomAccessIterator<Vector<EarTriangle*>*> i = ListPolygonTriangle.Begin(); i != ListPolygonTriangle.End(); i++)
    {
        Vector<EarTriangle*>* PolygonTriangles = *i;
        JSONArray polygon;
        for(RandomAccessIterator<EarTriangle*> j = PolygonTriangles->Begin(); j != PolygonTriangles->End(); j++)
        {
            EarTriangle* et = *j;
            JSONValue triangle;
            triangle.Set("p1_x_", JSONValue(et->p1_.x_));
            triangle.Set("p1_y_", JSONValue(et->p1_.y_));
            triangle.Set("p2_x_", JSONValue(et->p2_.x_));
            triangle.Set("p2_y_", JSONValue(et->p2_.y_));
            triangle.Set("p3_x_", JSONValue(et->p3_.x_));
            triangle.Set("p3_y_", JSONValue(et->p3_.y_));
            polygon.Push(triangle);
        }
        triangleArray.Push(JSONValue(polygon));
        MapNodeJson->Set("triangles",JSONValue(triangleArray));
    }

    JSONArray platformArray;// = MapNodeJson.CreateChild("platforms",JSON_ARRAY);
    for(RandomAccessIterator<PlatformData*> i = PlatformsList.Begin(); i != PlatformsList.End(); i++)
    {
        PlatformData* platData = *i;
        JSONValue platformDataJson;
        platformDataJson.Set("p1_x_", JSONValue(platData->p1.x_));
        platformDataJson.Set("p1_y_", JSONValue(platData->p1.y_));
        platformDataJson.Set("p2_x_", JSONValue(platData->p1.x_));
        platformDataJson.Set("p2_y_", JSONValue(platData->p1.y_));
        platformDataJson.Set("type", JSONValue(platData->type));
        platformArray.Push(platformDataJson);
    }

    JSONArray objectArray;// = MapNodeJson.CreateChild("objects",JSON_ARRAY);
    for(RandomAccessIterator<ObjectData*> i = ObjectList.Begin(); i != ObjectList.End(); i++)
    {
        ObjectData* objectData = *i;
        JSONValue objDataJson;
        objDataJson.Set("pos_x_", JSONValue(objectData->position.x_));
        objDataJson.Set("pos_y_", JSONValue(objectData->position.y_));
        objDataJson.Set("type", JSONValue(objectData->type));
        objDataJson.Set("code", JSONValue(objectData->Code));
        objectArray.Push(objDataJson);
    }

    File file(context_,GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/MapNode.json", FILE_WRITE);
    data->Save(file);

    /** Solo archivo de editor **/

    JSONFile* mapData = new JSONFile(context_);
    JSONValue* PolygonsJson = &mapData->GetRoot();
    Vector<Vector<PolygonVertex *>* > polygons = PolygonMap.Values();
    JSONArray jsonPolygonArray;
    for(RandomAccessIterator<Vector<PolygonVertex *>*> ps = polygons.Begin(); ps != polygons.End(); ps++)
    {
        Vector<PolygonVertex *>* polygon = *ps;
        JSONArray polygonJson;
        for(RandomAccessIterator<PolygonVertex*> pvi = polygon->Begin(); pvi != polygon->End(); pvi++)
        {
            PolygonVertex* pv = *pvi;
            JSONValue jsonPv;
            jsonPv.Set("x_",JSONValue(pv->GetVector().x_));
            jsonPv.Set("y_",JSONValue(pv->GetVector().y_));
            polygonJson.Push(jsonPv);
        }
        jsonPolygonArray.Push(JSONValue(polygonJson));
    }
    PolygonsJson->Set("polygons",JSONValue(jsonPolygonArray));
    File mapDataFile(context_,GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/MapData.json", FILE_WRITE);
    mapData->Save(mapDataFile);
}

void MapEditor::MoveCamera(float timeStep)
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
}

void MapEditor::SubscribeToEvents()
{
    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MapEditor, HandleUpdate));

    // Subscribe to mouse click
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(MapEditor, HandleMouseButtonDown));

    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(MapEditor, HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(MapEditor, HandleMouseButtonUp));
}

void MapEditor::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace Update;

    PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
    Input* input = GetSubsystem<Input>();

    if (input->GetKeyPress(KEY_SPACE))
        drawDebug_ = !drawDebug_;

    if (drawDebug_)
        physicsWorld->DrawDebugGeometry();

    if(input->GetKeyPress('T'))
        currentKeyFunction = TRASLATE;

    if(input->GetKeyPress('E'))
        currentKeyFunction = ADD;

    if(input->GetKeyPress('R'))
        currentKeyFunction = REMOVE;

    if(input->GetKeyPress('Z'))
        currentKeyFunction = NONE;

    if (input->GetKeyPress(KEY_F5))
        SaveMap();
    if (input->GetKeyPress(KEY_F7))
        LoadMap();

    float timeStep = eventData[P_TIMESTEP].GetFloat();

    MoveCamera(timeStep*2);

    CreateGrids();
    DrawPolygon();

    if (drawRectangle)
        DrawRectangle( Rect(dragPointBegin, dragPointEnd) );
}


void MapEditor::HandleMouseButtonDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    dragPointEnd = GetDiscreetPosition();
    dragPointBegin = Vector2(dragPointEnd.x_, dragPointEnd.y_+0.7f);

    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    switch (currentFunction)
    {
        case DRAWBODY:
            bodyFunctions();
            break;
        case DRAWCHAR:
            if(currentCharType == ENEMY)
            {
                if(currentKeyFunction == ADD)
                    CreateEnemy(GetDiscreetPosition()+Vector2(0.35f,0.35f));
                if(currentKeyFunction == REMOVE)
                {
                    PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
                    RigidBody2D* rigidBody = physicsWorld->GetRigidBody(GetMousePositionXY());
                    if (rigidBody)
                    {
                        Node* removenode = rigidBody->GetNode();
                        if(removenode->GetName() == "enemy")
                        {
                            ObjectData* data = removenode->GetComponent<ObjectData>();
                            ObjectList.Remove(data);
                        }
                        removenode->Remove();
                    }
                }
            }
            break;
    }

    /*SubscribeToEvent(E_MOUSEMOVE, HANDLER(MapEditor, HandleMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONUP, HANDLER(MapEditor, HandleMouseButtonUp));*/
}


void MapEditor::HandleMouseMove(StringHash eventType, VariantMap& eventData)
{
    switch (currentFunction)
    {
        case DRAWBODY:
            switch (currentBodyType)
            {
            case MOVPLATFORM:
                if(currentpd)
                {
                    currentpd->imagereference->SetPosition2D(GetDiscreetPosition()+Vector2(0.7f,+0.6f));
                    currentpd->p2 = GetDiscreetPosition()+Vector2(0.7f,+0.6f);
                }
                break;
            case MIDLEPLATFORM:
            case PLATFORM:
                dragPointEnd.x_ = GetDiscreetPosition().x_;
                break;
            case VERTEXPOLYGON:
                if(currentKeyFunction == TRASLATE)
                    if(selectObject_)
                        CurrentVertex->SetVector(GetDiscreetPosition());
                break;
            }
            break;
        case DRAWENV:
            break;
        case DRAWCHAR:
            if(currentCharType == PLAYER)
            {
                if(currentKeyFunction == TRASLATE)
                    nodePlayer->SetPosition2D(GetDiscreetPosition()+Vector2(0.35f,0.35f));
            }
            break;
    }
}

void MapEditor::HandleMouseButtonUp(StringHash eventType, VariantMap& eventData)
{
    if (!GetSubsystem<UI>()->GetFocusElement())
    {
        if(currentKeyFunction == ADD)
        {
            drawRectangle = false;
            if(currentBodyType == PLATFORM)
                CreatePlatform(dragPointBegin, dragPointEnd, "platform");
            if(currentBodyType == MIDLEPLATFORM)
                CreatePlatform(dragPointBegin, dragPointEnd, "midleplatform");
        }
    }
    currentpd = 0;

    if(currentKeyFunction == ADD)
        return;

    currentKeyFunction = NONE;
/*
    UnsubscribeFromEvent(E_MOUSEMOVE);
    UnsubscribeFromEvent(E_MOUSEBUTTONUP);*/
}

void MapEditor::bodyFunctions()
{
    Node* nodevertex;
    PolygonVertex * cvertex;
    Node* removenode;

    switch (currentBodyType)
    {
    case MOVPLATFORM:
        if(currentKeyFunction == ADD)
        {
            CreateMovablePlatform(GetDiscreetPosition()+Vector2(0.7f,+0.6f),GetDiscreetPosition()+Vector2(0.7f,+0.6f));
        }
        if(currentKeyFunction == REMOVE)
        {
            PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
            RigidBody2D* rigidBody = physicsWorld->GetRigidBody(GetMousePositionXY());
            if (rigidBody)
            {
                removenode = rigidBody->GetNode();
                if(removenode->GetName() == "movplatform")
                {
                    PlatformData* platdata = removenode->GetComponent<PlatformData>();
                    platdata->imagereference->Remove();
                    PlatformsList.Remove(platdata);
                    removenode->Remove();
                }
            }
        }
        break;
    case MIDLEPLATFORM:
    case PLATFORM:
        if(currentKeyFunction == ADD)
        {
            drawRectangle = true;
        }
        if(currentKeyFunction == REMOVE)
        {
            PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
            RigidBody2D* rigidBody = physicsWorld->GetRigidBody(GetMousePositionXY());
            if (rigidBody)
            {
                removenode = rigidBody->GetNode();
                if(removenode->GetName() == "wall")
                {
                    PlatformData* platdata = removenode->GetComponent<PlatformData>();
                    PlatformsList.Remove(platdata);
                    platdata->imagereference->Remove();
                    removenode->Remove();
                }
            }
        }
        break;
    case POLYGONBODY:
        if(currentKeyFunction == ADD)
        {
            PolygonMap.Insert(Pair<String, Vector<PolygonVertex *>*>("Polygon" + String(PolygonCounter), CreatePolygon()));
            PolygonCounter++;
            LoadPolygonList();
        }
        if(currentKeyFunction == REMOVE)
        {
            PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
            RigidBody2D* rigidBody = physicsWorld->GetRigidBody(GetMousePositionXY());
            if (rigidBody)
            {
                PolygonVertex * p = rigidBody->GetComponent<PolygonVertex>();
                RemovePolygon(p);
            }
        }
        break;
    case VERTEXPOLYGON:
        PhysicsWorld2D* physicsWorld = scene_->GetComponent<PhysicsWorld2D>();
        RigidBody2D* rigidBody = physicsWorld->GetRigidBody(GetMousePositionXY());
        switch (currentKeyFunction)
        {
        case NONE:
            if(rigidBody)
            {
                if(selectObject_)
                    CurrentVertex->setUnselect();
                CurrentVertex = rigidBody->GetComponent<PolygonVertex>();
                if(CurrentVertex)
                {
                    CurrentVertex->setSelect();
                    selectObject_ = true;
                }
            }
            else{
                selectObject_ = false;
                if(CurrentVertex)
                    CurrentVertex->setUnselect();
            }
            break;
        case REMOVE:
            if(rigidBody)
            {
                CurrentVertex = rigidBody->GetComponent<PolygonVertex>();
                if(CurrentVertex)
                {
                    if(CurrentPolygon)
                    {
                        if(CurrentPolygon->Remove(*(CurrentPolygon->Find(CurrentVertex))))
                        {
                            removenode = rigidBody->GetNode();
                            removenode->Remove();
                            selectObject_ = false;
                        }
                    }
                }
            }
            break;
        case ADD:
            if(CurrentPolygon)
            {
                if(selectObject_)
                {
                    nodevertex = scene_->CreateChild("vertex");
                    cvertex = nodevertex->CreateComponent<PolygonVertex>();
                    cvertex->SetVector(GetDiscreetPosition());
                    insertVertex(CurrentPolygon, cvertex);
                }

            }
            break;
        case TRASLATE:
            if(!selectObject_)
            {
                currentKeyFunction = NONE;
                bodyFunctions();
            }
            break;
        }
        break;
    }
}

void MapEditor::DrawRectangle(Rect rect)
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

void MapEditor::CreateEnemy(Vector2 p1)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    Node* enemynode = nodeWall->CreateChild("enemy");

    Sprite2D* vertexsprite = cache->GetResource<Sprite2D>("Urho2D/object.png");
    if (!vertexsprite)
        return;
	StaticSprite2D* staticSprite = enemynode->CreateComponent<StaticSprite2D>();
	staticSprite->SetSprite(vertexsprite);
	staticSprite->SetLayer(60000);
	staticSprite->SetColor(Color(Color::RED,1));

    RigidBody2D* bodysprite = enemynode->CreateComponent<RigidBody2D>();
    bodysprite->SetBodyType(BT_STATIC);

    CollisionCircle2D* circle = enemynode->CreateComponent<CollisionCircle2D>();
    circle->SetRadius(0.2f);

    ObjectData* data = enemynode->CreateComponent<ObjectData>();
    data->type = "enemy";
    data->Code = "t01";
    data->SetPostion(p1);

    ObjectList.Push(data);
}

void MapEditor::CreatePlatform(Vector2 p1, Vector2 p2, String typePlatform)
{
    float mwith = fabs(p2.x_ - p1.x_)/2;
    if(mwith == 0)
        return;
    float mheigth = 0.35f;
    Vector2 pos((p2.x_ + p1.x_)/2, (p2.y_ + p1.y_)/2);

    Node* node  = nodeWall->CreateChild("wall");
    node->SetPosition2D(pos);

    PlatformData* platData = node->CreateComponent<PlatformData>();
    platData->p1 = p1;
    platData->p2 = p2;
    platData->type = typePlatform;
    PlatformsList.Push(platData);

    RigidBody2D* body = node->CreateComponent<RigidBody2D>();
    body->SetBodyType(BT_STATIC);

    PODVector<Vector2> vertices;
    if(typePlatform == "platform")
    {
        vertices.Push(Vector2(-mwith,mheigth/2));
        vertices.Push(Vector2(mwith,mheigth/2));
        vertices.Push(Vector2(mwith,-mheigth));
        vertices.Push(Vector2(-mwith,-mheigth));
    }
    else{
        vertices.Push(Vector2(-mwith,mheigth/2));
        vertices.Push(Vector2(mwith,mheigth/2));
        vertices.Push(Vector2(mwith,0));
        vertices.Push(Vector2(-mwith,0));
    }

    CollisionPolygon2D* box = node->CreateComponent<CollisionPolygon2D>();
    box->SetVertices(vertices);
    box->SetDensity(1.0f);
    box->SetFriction(0.0f);
    box->SetRestitution(0.1f);
    box->SetCategoryBits(32768);

    Node* pnode  = nodeWall->CreateChild("platform");
    pnode->SetPosition2D(pos);
    RigidBody2D* pbody = pnode->CreateComponent<RigidBody2D>();
    pbody->SetBodyType(BT_STATIC);

    PODVector<Vector2> pvertices;
    pvertices.Push(Vector2(-mwith,mheigth));
    pvertices.Push(Vector2(mwith,mheigth));
    pvertices.Push(Vector2(mwith,mheigth/2));
    pvertices.Push(Vector2(-mwith,mheigth/2));

    CollisionPolygon2D* pbox = pnode->CreateComponent<CollisionPolygon2D>();
    pbox->SetVertices(pvertices);
    pbox->SetDensity(1.0f);
    pbox->SetFriction(0.5f);
    pbox->SetRestitution(0.1f);
    pbox->SetCategoryBits(32768);

    platData->imagereference = pnode;
}

void MapEditor::CreateMovablePlatform(Vector2 p1, Vector2 p2)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    PODVector<Vector2> vertices;
    vertices.Push(Vector2(-0.7f,0.1f));
    vertices.Push(Vector2(0.7f,0.1f));
    vertices.Push(Vector2(0.7f,-0.1f));
    vertices.Push(Vector2(-0.7f,-0.1f));

    Node* movplatformnode  = nodeWall->CreateChild("movplatform");
    movplatformnode->SetPosition2D(p1);

    Sprite2D* movplatformsprite = cache->GetResource<Sprite2D>("Urho2D/movplatform.png");
    if (!movplatformsprite)
        return;

    StaticSprite2D* movplatformstaticSprite = movplatformnode->CreateComponent<StaticSprite2D>();
    movplatformstaticSprite->SetSprite(movplatformsprite);

    RigidBody2D* platfotmbody = movplatformnode->CreateComponent<RigidBody2D>();
    platfotmbody->SetBodyType(BT_KINEMATIC);
    platfotmbody->SetFixedRotation(true);

    CollisionPolygon2D* box = movplatformnode->CreateComponent<CollisionPolygon2D>();
    box->SetVertices(vertices);
    box->SetDensity(1.0f);
    box->SetFriction(0.5f);
    box->SetRestitution(0.0f);
    box->SetCategoryBits(32768);

    PlatformData* platdata = movplatformnode->CreateComponent<PlatformData>();
    platdata->type = "movplatform";
    platdata->p1 = p1;

    PlatformsList.Push(platdata);
    currentpd = platdata;

    Node* movplatformreference = nodeWall->CreateChild("reference");

    StaticSprite2D* platformref = movplatformreference->CreateComponent<StaticSprite2D>();
    platformref->SetSprite(movplatformsprite);
    movplatformreference->SetPosition2D(p2);
    currentpd->p2 = p2;
    currentpd->imagereference = movplatformreference;
}

void MapEditor::DrawPolygon()
{
    DebugRenderer* debug = scene_->GetComponent<DebugRenderer>();

    Vector<Vector<PolygonVertex *>* > polygons = PolygonMap.Values();
    for(RandomAccessIterator<Vector<PolygonVertex *>*> ps = polygons.Begin(); ps != polygons.End(); ps++)
    {
        Vector<PolygonVertex *>* polygon = *ps;
        if(!polygon->Empty())
        {
            int i = 1;
            for(i; i < polygon->Size(); i++)
            {
                Vector2 p1 = (polygon->operator[](i-1))->GetVector();
                Vector2 p2 = (polygon->operator[](i))->GetVector();
                debug->AddLine(Vector3(p1.x_, p1.y_, 0), Vector3(p2.x_, p2.y_, 0), Color(1, 0, 0, 0),  false);
            }
            Vector2 p1 = (polygon->operator[](i-1))->GetVector();
            Vector2 p2 = (polygon->operator[](0))->GetVector();
            debug->AddLine(Vector3(p1.x_, p1.y_, 0), Vector3(p2.x_, p2.y_, 0), Color(1, 0, 0, 0),  false);
        }
    }

    for(RandomAccessIterator<Vector<EarTriangle*>*> i = ListPolygonTriangle.Begin(); i != ListPolygonTriangle.End(); i++)
    {
        Vector<EarTriangle*>* PolygonTriangles = *i;
        for(RandomAccessIterator<EarTriangle*> j = PolygonTriangles->Begin(); j != PolygonTriangles->End(); j++)
        {
            EarTriangle* et = *j;

            debug->AddLine(Vector3(et->p1_.x_, et->p1_.y_, 0), Vector3(et->p2_.x_, et->p2_.y_, 0), Color(0, 1, 0, 0),  false);
            debug->AddLine(Vector3(et->p2_.x_, et->p2_.y_, 0), Vector3(et->p3_.x_, et->p3_.y_, 0), Color(0, 1, 0, 0),  false);
            debug->AddLine(Vector3(et->p3_.x_, et->p3_.y_, 0), Vector3(et->p1_.x_, et->p1_.y_, 0), Color(0, 1, 0, 0),  false);
        }
    }
}

void MapEditor::DrawCharacter()
{
}

Vector2 MapEditor::GetMousePositionXY()
{
    Input* input = GetSubsystem<Input>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Vector3 screenPoint = Vector3((float)input->GetMousePosition().x_ / graphics->GetWidth(), (float)input->GetMousePosition().y_ / graphics->GetHeight(), 0.0f);

    Vector3 worldPoint = camera_->ScreenToWorldPoint(screenPoint);
    return Vector2(worldPoint.x_, worldPoint.y_);
}

Vector2 MapEditor::GetDiscreetPosition()
{
    Vector2 discreetposition = GetMousePositionXY();
    discreetposition.x_ = (floor(discreetposition.x_/0.7f) * 0.7f);
    discreetposition.y_ = (floor(discreetposition.y_/0.7f) * 0.7f);

    return discreetposition;
}

void MapEditor::InitWindow()
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
    ListView* seconditemlist = (ListView*)auxwindow->GetChild("SecondList",true);

    JSONFile* data = new JSONFile(context_);
    File file(context_,"Data/Scenes/map_editor.json");
    data->Load(file);

    rootjson = data->GetRoot();

    DropDownList* DropDownType = (DropDownList*)auxwindow->GetChild("ObjeList",true);
    Text* SelectedText = static_cast<Text*>(DropDownType->GetSelectedItem());
    LoadSelectedType(SelectedText->GetText());

    Button* button = (Button*)auxwindow->GetChild("ProcessButton", true);

    SubscribeToEvent(DropDownType, E_ITEMSELECTED, URHO3D_HANDLER(MapEditor, HandleChangeType));
    SubscribeToEvent(itemlist, E_ITEMSELECTED, URHO3D_HANDLER(MapEditor, HandleLoadPreview));
    SubscribeToEvent(seconditemlist, E_ITEMSELECTED, URHO3D_HANDLER(MapEditor, HandleSelectSecondList));
	SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MapEditor, HandleProcess));
}

void MapEditor::HandleChangeType(StringHash eventType, VariantMap& eventData)
{
    DropDownList* DropDownType = static_cast<DropDownList*>(eventData["Element"].GetPtr());
    Text* SelectedText = static_cast<Text*>(DropDownType->GetSelectedItem());
    LoadSelectedType(SelectedText->GetText());
}

void MapEditor::HandleSelectSecondList(StringHash eventType, VariantMap& eventData)
{
    ListView* ItemList = static_cast<ListView*>(eventData["Element"].GetPtr());
    Text* SelectedText = static_cast<Text*>(ItemList->GetSelectedItem());
    //currentBodyType
    switch (currentFunction)
    {
    case DRAWBODY:
        UnselectPolygon(CurrentPolygon);
        CurrentPolygon = PolygonMap[SelectedText->GetText()];
        SelectPolygon(CurrentPolygon);
        switch (currentBodyType)
        {
        case VERTEXPOLYGON:
            break;
        }
        break;
    }
}

void MapEditor::HandleLoadPreview(StringHash eventType, VariantMap& eventData)
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

    if(CurrentType == "Body")
    {
        currentKeyFunction = NONE;
        ListView* ItemList = static_cast<ListView*>(eventData["Element"].GetPtr());
        Text* SelectedText = static_cast<Text*>(ItemList->GetSelectedItem());
        typebody = SelectedText->GetText();
        if(SelectedText->GetText() == "Platform")
        {
            currentBodyType = PLATFORM;
        }
        if(SelectedText->GetText() == "MidlePlatform")
        {
            currentBodyType = MIDLEPLATFORM;
        }
        if(SelectedText->GetText() == "MovPlatform")
        {
            currentBodyType = MOVPLATFORM;
        }
        if(SelectedText->GetText() == "PolygonBody")
        {
            currentBodyType = POLYGONBODY;
        }
        if(SelectedText->GetText() == "VertexPolygon")
        {
            currentBodyType = VERTEXPOLYGON;
        }
    }
}

void MapEditor::SelectPolygon(Vector<PolygonVertex *>* polygon)
{
    if(!polygon)
        return;
    for(int i = 0; i < polygon->Size(); i++)
    {
       (polygon->operator[](i))->setSelectPolygon();
    }
}

void MapEditor::UnselectPolygon(Vector<PolygonVertex *>* polygon)
{
    if(!polygon)
        return;
    for(int i=0; i < polygon->Size(); i++)
    {
       (polygon->operator[](i))->setUnselect();
    }
}

void MapEditor::LoadPolygonList()
{
    ListView* seconditemlist = (ListView*)window_->GetChild("SecondList",true);
    seconditemlist->RemoveAllItems();
    Vector<String> keys = PolygonMap.Keys();
    for( int i = 0 ; i < keys.Size() ; i++ )
    {
        Text* item = new Text(context_);
        item->SetText(keys[i]);
        item->SetStyle("FileSelectorListText");
        seconditemlist->InsertItem(seconditemlist->GetNumItems(), item);
    }
}

void MapEditor::LoadSelectedType(String type)
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
        JSONValue jsonType = rootjson.Get(type);
        JSONArray jsonTypeArray = jsonType.GetArray();
        for(int i = 0 ; i < jsonTypeArray.Size() ; i++)
        {
            Text* item = new Text(context_);
            item->SetText(jsonType[i].GetString());
            item->SetStyle("FileSelectorListText");
            itemlist->InsertItem(itemlist->GetNumItems(), item);
        }
    }
    if(type == "Body")
    {
        currentFunction = DRAWBODY;
        LoadPolygonList();
    }

    if(type == "Characters")
        currentFunction = DRAWCHAR;
    if(type == "Escenario")
        currentFunction = DRAWENV;
}

bool MapEditor::RemovePolygon(PolygonVertex * p)
{
    Vector<String> keys = PolygonMap.Keys();
    for(int i = 0; i < keys.Size(); i++)
    {
        Vector<PolygonVertex *>* polygon = PolygonMap[keys[i]];
        if(polygon->Contains(p))
        {
            while(!polygon->Empty())
            {
                PolygonVertex * pv = polygon->Back();
                Node* noderemove = pv->GetNode();
                noderemove->Remove();
                polygon->Pop();
            }
            PolygonMap.Erase(keys[i]);
        }
    }
    UnselectPolygon(CurrentPolygon);
    CurrentPolygon = 0;
    LoadPolygonList();
}

bool MapEditor::RemovePolygon(String key)
{
    Vector<PolygonVertex *>* polygon = PolygonMap[key];
    while(!polygon->Empty())
    {
        PolygonVertex * pv = polygon->Back();
        Node* noderemove = pv->GetNode();
        noderemove->Remove();
        polygon->Pop();
    }
    PolygonMap.Erase(key);
    UnselectPolygon(CurrentPolygon);
    CurrentPolygon = 0;
    LoadPolygonList();
}

Vector<PolygonVertex *>* MapEditor::CreatePolygon()
{
    Vector<PolygonVertex *>* polygon_ = new Vector<PolygonVertex *>();

    Vector2 pos = GetDiscreetPosition();
    polygon_->Push(CreatePolygonVertex(Vector2(pos)));
    polygon_->Push(CreatePolygonVertex(Vector2(pos.x_,pos.y_+0.7f)));
    polygon_->Push(CreatePolygonVertex(Vector2(pos.x_+0.7f,pos.y_+0.7f)));
    polygon_->Push(CreatePolygonVertex(Vector2(pos.x_+0.7f,pos.y_)));

    UnselectPolygon(CurrentPolygon);
    CurrentPolygon = polygon_;
    SelectPolygon(polygon_);
    return polygon_;
}

PolygonVertex * MapEditor::CreatePolygonVertex(Vector2 pos)
{
    Node* nv = scene_->CreateChild("vertex");
    PolygonVertex * pv = nv->CreateComponent<PolygonVertex>();
    pv->SetVector(pos);
    return pv;
}

void MapEditor::insertVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * newvertex)
{
    if(!CurrentVertex)
        return;
    unsigned index = 0;
    for(RandomAccessIterator<PolygonVertex*> v = polygon->Begin(); v != polygon->End(); v++)
    {
        if(polygon->Find(CurrentVertex) == v)
            break;
        index++;
    }
    CurrentVertex->setSelectPolygon();
    polygon->Insert(index, newvertex);
    CurrentVertex = newvertex;
    CurrentVertex->setSelect();
}

/* Process polygon */

void MapEditor::HandleProcess(StringHash eventType, VariantMap& eventData)
{
    ListPolygonTriangle.Clear();
    Vector<Vector<PolygonVertex *>* > polygons = PolygonMap.Values();
    for(RandomAccessIterator<Vector<PolygonVertex *>*> ps = polygons.Begin(); ps != polygons.End(); ps++)
    {
        Vector<PolygonVertex *>* polygon = *ps;
        Vector<EarTriangle*>* PolygonTriagles = new Vector<EarTriangle*>();
        ListPolygonTriangle.Push(PolygonTriagles);
        cleanVertex(polygon);
        if(!polygon->Empty() || polygon->Size() > 3)
        {
            PolygonVertex * p = polygon->operator[](0);
            int counter = polygon->Size();
            while( counter > 3)
            {
                p->isEvalue = true;
                Vector2 p1 = prevVertex(polygon,p);
                Vector2 p2 = p->GetVector();
                Vector2 p3 = nextVertex(polygon,p);
                if(!ccw(p1, p2, p3))
                {
                    bool noear = false;
                    for(int j = 0; j < polygon->Size(); j++)
                    {
                        PolygonVertex * testpolygon = polygon->operator[](j);
                        if(!testpolygon->isEvalue && !testpolygon->isProcess)
                        {
                            if(isInTriangle(testpolygon->GetVector(),p1,p2,p3))
                            {
                                noear = true;
                                break;
                            }
                        }
                    }
                    if(!noear)
                    {
                        PolygonTriagles->Push(new EarTriangle(p1,p2,p3));
                        p->isProcess = true;
                        counter--;
                    }
                }
                p->isEvalue = false;
                CurrentPrevVertex->isEvalue = false;
                CurrentNextVertex->isEvalue = false;
                p = CurrentNextVertex;

            }
            Vector<Vector2> lasttriangle;
            for(int j = 0; j < polygon->Size(); j++)
            {
                if(!(polygon->operator[](j))->isProcess)
                    lasttriangle.Push((polygon->operator[](j))->GetVector());
            }
            PolygonTriagles->Push(new EarTriangle(lasttriangle[0],lasttriangle[1],lasttriangle[2]));
        }
    }
    ProcessPolygonPhysics();
    Button* processbutton = static_cast<Button*>(eventData["Element"].GetPtr());
    processbutton->SetFocus(false);
}

void MapEditor::ProcessPolygonPhysics()
{

    while(!ListNodePolygonsPhysics.Empty())
    {
        Node* noderemove = ListNodePolygonsPhysics.Back();
        if(noderemove)
        {
            noderemove->Remove();
            ListNodePolygonsPhysics.Pop();
        }
    }
    for(RandomAccessIterator<Vector<EarTriangle*>*> i = ListPolygonTriangle.Begin(); i != ListPolygonTriangle.End(); i++)
    {
        Node* polygonnode = scene_->CreateChild("Wall");
        ListNodePolygonsPhysics.Push(polygonnode);
        RigidBody2D* body = polygonnode->CreateComponent<RigidBody2D>();
        body->SetBodyType(BT_STATIC);

        Vector<EarTriangle*>* PolygonTriangles = *i;
        for(RandomAccessIterator<EarTriangle*> j = PolygonTriangles->Begin(); j != PolygonTriangles->End(); j++)
        {
            EarTriangle* et = *j;

            PODVector<Vector2> vertices;
            vertices.Push(et->p1_);
            vertices.Push(et->p2_);
            vertices.Push(et->p3_);

            CollisionPolygon2D* triangle = polygonnode->CreateComponent<CollisionPolygon2D>();
            triangle->SetVertices(vertices);
            triangle->SetDensity(1.0f);
            triangle->SetFriction(0.0f);
            triangle->SetRestitution(0.1f);
            triangle->SetCategoryBits(32768);
        }
    }
}

void MapEditor::cleanVertex(Vector<PolygonVertex *>* polygon)
{
    for(int k = 0; k < polygon->Size(); k++)
    {
        (polygon->operator[](k))->isEvalue = false;
        (polygon->operator[](k))->isProcess = false;
    }
}

Vector2 MapEditor::nextVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * P)
{
    for(RandomAccessIterator<PolygonVertex*> p_ = polygon->Find(P)+1; p_ != polygon->End(); p_++)
    {
        PolygonVertex * pv = *p_;
        if(!pv->isProcess)
        {
            CurrentNextVertex = pv;
            pv->isEvalue = true;
            return  pv->GetVector();
        }
    }
    for(RandomAccessIterator<PolygonVertex*> p_ = polygon->Begin(); p_ != polygon->Find(P); p_++)
    {
        PolygonVertex * pv = *p_;
        if(!pv->isProcess)
        {
            CurrentNextVertex = pv;
            pv->isEvalue = true;
            return  pv->GetVector();
        }
    }
    return Vector2::ZERO;
}

Vector2 MapEditor::prevVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * P)
{
    if(polygon->Find(P)!= polygon->Begin())
    {
        for(RandomAccessIterator<PolygonVertex*> p_ = polygon->Find(P)-1; p_ >= polygon->Begin(); p_--)
        {
            PolygonVertex * pv = *p_;
            if(!pv->isProcess)
            {
                CurrentPrevVertex = pv;
                pv->isEvalue = true;
                return  pv->GetVector();
            }
        }
    }

    for(RandomAccessIterator<PolygonVertex*> p_ = polygon->End()-1; p_ != polygon->Find(P); p_--)
    {
        PolygonVertex * pv = *p_;
        if(!pv->isProcess)
        {
            CurrentPrevVertex = pv;
            pv->isEvalue = true;
            return  pv->GetVector();
        }
    }
    return Vector2::ZERO;
}

bool MapEditor::ccw(Vector2 p1, Vector2 p2, Vector2 p3)
{
    return ((p1.x_ - p2.x_) * (p3.y_ - p2.y_) - (p1.y_ - p2.y_) * (p3.x_ - p2.x_)) < 0;;
}

float MapEditor::Sign (Vector2 p1, Vector2 p2, Vector2 p3)
{
    return (p1.x_ - p3.x_) * (p2.y_ - p3.y_) - (p2.x_ - p3.x_) * (p1.y_ - p3.y_);
}

bool MapEditor::isInTriangle (Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3)
{
    bool b1, b2, b3;

    b1 = Sign(pt, v1, v2) < 0.0f;
    b2 = Sign(pt, v2, v3) < 0.0f;
    b3 = Sign(pt, v3, v1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

/* End Process polygon */
