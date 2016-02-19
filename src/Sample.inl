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

#include "Urho3D/Engine/Application.h"
#include "Urho3D/Graphics/Camera.h"
#include "Urho3D/Engine/Console.h"
#include "Urho3D/UI/Cursor.h"
#include "Urho3D/Engine/Engine.h"
#include "Urho3D/IO/FileSystem.h"
#include "Urho3D/Graphics/Graphics.h"
#include "Urho3D/Input/Input.h"
#include "Urho3D/Input/InputEvents.h"
#include "Urho3D/Graphics/Renderer.h"
#include "Urho3D/Resource/ResourceCache.h"
#include "Urho3D/Scene/Scene.h"
#include "Urho3D/Scene/SceneEvents.h"
#include "Urho3D/UI/Sprite.h"
#include "Urho3D/Graphics/Texture2D.h"
#include "Urho3D/Core/Timer.h"
#include "Urho3D/UI/UI.h"
#include "Urho3D/Resource/XMLFile.h"

Sample::Sample(Context* context) :
    Application(context)
{
}

void Sample::Setup()
{
    // Modify engine startup parameters
    engineParameters_["WindowTitle"] = "Map Editor";
    engineParameters_["LogName"]     = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
    engineParameters_["FullScreen"]  = false;
    engineParameters_["Headless"]    = false;
    engineParameters_["WindowWidth"] = 1440;
    engineParameters_["WindowHeight"]= 800;
    engineParameters_["VSync"] = true;
}

void Sample::Start()
{
    // Set custom window Title & Icon
    SetWindowTitleAndIcon();

    // Subscribe key down event
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Sample, HandleKeyDown));
}

void Sample::SetWindowTitleAndIcon()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Image* icon = cache->GetResource<Image>("Textures/UrhoIcon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("Urho3D Sample");
}

void Sample::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();

    // Close console (if open) or exit when ESC is pressed
    if (key == KEY_ESC)
    {
        engine_->Exit();
    }
}
