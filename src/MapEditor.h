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

#pragma once

#include "Sample.h"
#include "Urho3D/Urho2D/CollisionPolygon2D.h"
#include "Urho3D/Container/LinkedList.h"
#include "PolygonVertex.h"

namespace Urho3D
{

class Window;
class Node;
class Scene;
class Sprite;
}

enum Function
{
    DRAWBODY,
    DRAWCHAR,
    DRAWENV
};

enum EnvLayer
{
    FLOOR,
    TOP,
};

enum TypeCharacter
{
    PLAYER,
    ENEMY,
    NPC
};

enum TypeBody
{
    PLATFORM,
    POLYGONBODY,
    VERTEXPOLYGON,
    MIDLEPLATFORM,
    MOVPLATFORM
};

enum TypeKeyFunction
{
    NONE,
    TRASLATE,
    ADD,
    REMOVE
};

struct EarTriangle
{
    EarTriangle(Vector2 p1,Vector2 p2,Vector2 p3)
    {
        p1_ = p1;
        p2_ = p2;
        p3_ = p3;
    }
    Vector2 p1_,p2_,p3_;
};

Vector2  dragPointBegin;
Vector2  dragPointEnd;
bool     drawRectangle = false;

class MapEditor : public Sample
{
    URHO3D_OBJECT(MapEditor, Sample);

public:
    MapEditor(Context* context);
    virtual void Start();

private:
    void HandleMouseMove(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseButtonUp(StringHash eventType, VariantMap& eventData);

    void CreateGrids();
    void DrawRectangle(Rect rect);
    void CreatePlatform(Vector2 p1, Vector2 p2, String typeplatform);
    void CreateMovablePlatform(Vector2 p1, Vector2 p2);
    void CreateEnemy(Vector2 p1);
    void DrawWall(int button);

    void DrawCharacter();

    void CreateScene();
    void CreatePreviewScene();
    void InitWindow();

    void HandleChangeType(StringHash eventType, VariantMap& eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleLoadPreview(StringHash eventType, VariantMap& eventData);
    void HandleProcess(StringHash eventType, VariantMap& eventData);
    void HandleSelectSecondList(StringHash eventType, VariantMap& eventData);

    void SetupViewport();
    void MoveCamera(float timeStep);
    void SubscribeToEvents();

    void LoadMap();
    void SaveMap();

    void LoadSelectedType(String type);

    void DrawPolygon();

    void ProcessPolygonPhysics();

    void bodyFunctions();

    void SelectPolygon(Vector<PolygonVertex *>* polygon);

    void UnselectPolygon(Vector<PolygonVertex *>* polygon);

    float Sign (Vector2 p1, Vector2 p2, Vector2 p3);

    bool isInTriangle (Vector2 pt, Vector2 v1, Vector2 v2, Vector2 v3);

    bool ccw (Vector2 p1, Vector2 p2, Vector2 p3);

    Vector2 nextVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * P);
    Vector2 prevVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * P);

    void cleanVertex(Vector<PolygonVertex *>* polygon);

    void insertVertex(Vector<PolygonVertex *>* polygon, PolygonVertex * newvertex);

    Vector<PolygonVertex *>* CreatePolygon();

    PolygonVertex * CreatePolygonVertex(Vector2 pos);

    bool RemovePolygon(PolygonVertex * p);
    bool RemovePolygon(String key);

    void LoadPolygonList();

    /// The Window.
    SharedPtr<Window> window_;
    /// The UI's root UIElement.
    SharedPtr<UIElement> uiRoot_;
    /// Remembered drag begin position.
    IntVector2 dragBeginPosition_;
    /// Sprite nodes.
    Vector<SharedPtr<Node> > spriteNodes_;
    /// Scene preview object.
    SharedPtr<Scene> objprev_scene;
    /// Objetct preview camera scene node.
    SharedPtr<Node> ObjPrevCameraNode_;

    PlatformData* currentpd;
    SharedPtr<TileMap2D> tileMap;

    JSONValue rootjson;

    int PolygonCounter = 0;
    HashMap< String, SharedPtr< Sprite2D > > TileSetMap;
    HashMap< String, Vector<PolygonVertex *>* > PolygonMap;

    String CurrentType;

    /// Get mouse position in 2D world coordinates.
    Vector2 GetMousePositionXY();

    Vector2 GetDiscreetPosition();

    Function currentFunction = DRAWBODY;
    TypeCharacter currentCharType = PLAYER;
    TypeBody currentBodyType = POLYGONBODY;
    TypeKeyFunction currentKeyFunction = NONE;

    /// Flag for drawing debug geometry.
    bool drawDebug_;
    bool selectObject_ = false;
    /// Camera object.
    Camera* camera_;

    Vector<EarTriangle*> earTriagles;
    Vector< Vector<EarTriangle*>* > ListPolygonTriangle;
    Vector<Node*> ListNodePolygonsPhysics;
    Vector<Node*> CuadrilateralPhysics;
    Vector<PlatformData*> PlatformsList;
    Vector<ObjectData*> ObjectList;

    SharedPtr<Node> nodeWall;
    SharedPtr<Node> nodePlayer;
    Vector<Node*> EnemyList;
    Vector<Node*> NPCyList;
    String typebody = "Wall";


    Vector2 prevPositionLayer;
    Vector<PolygonVertex *>* CurrentPolygon;
    Vector< Vector<PolygonVertex *>* > ListPolygon;
    PolygonVertex * CurrentVertex;
    PolygonVertex * CurrentPrevVertex;
    PolygonVertex * CurrentNextVertex;

};


