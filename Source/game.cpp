#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Cursor.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/IO/Log.h>

#include "game.h"


Game::Game(Context *context) : Application(context)
{
}

void Game::Setup()
{
	engineParameters_[EP_FULL_SCREEN] = false;
    engineParameters_[EP_WINDOW_HEIGHT] = 768;
    engineParameters_[EP_WINDOW_WIDTH] = 1024;
    // Resource prefix path is a list of semicolon-separated paths which will be checked for containing resource directories. They are relative to application executable file.
    engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ".;..";
	engineParameters_[EP_WINDOW_MAXIMIZE] = true;
	engineParameters_[EP_WINDOW_RESIZABLE]=true;
}

void Game::Start()
{
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Game, HandleKeyDown));
	SubscribeToEvent(StringHash("Update"), URHO3D_HANDLER(Game, HandleUpdate));
	auto cache = GetSubsystem<ResourceCache>();
	auto ui = GetSubsystem<UI>();
	
	auto input = GetSubsystem<Input>();
	input->SetMouseVisible(true);
	
	scene_=new Scene(context_);
	scene_->CreateComponent<Octree>();
		
	cameranode_=scene_->CreateChild();
	Camera *camera=cameranode_->CreateComponent<Camera>();
	camera->SetFov(50.f);
	camera->SetViewMask(3);
		
	cameranode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
	
	cameranode_->Translate(Vector3::BACK * 10);
		
	auto* renderer = GetSubsystem<Renderer>();
	
	auto* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    SharedPtr<Cursor> cursor(new Cursor(context_));
    cursor->SetStyleAuto(style);
    ui->SetCursor(cursor);

    // Set starting position of the cursor at the rendering window center
    auto* graphics = GetSubsystem<Graphics>();
    cursor->SetPosition(graphics->GetWidth() / 2, graphics->GetHeight() / 2);

	// Set up a viewport to the Renderer subsystem so that the 3D scene can be seen
	SharedPtr<Viewport> viewport(new Viewport(context_, scene_, camera));
	renderer->SetViewport(0, viewport);
	
	Node* zoneNode = scene_->CreateChild("Zone");
	Zone *zone = zoneNode->CreateComponent<Zone>();
	zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
	zone->SetAmbientColor(Color(0.25f, 0.25f, 0.25f));
	zone->SetFogColor(Color(0.2, 0.2, 0.3));
	zone->SetFogStart(200.0f);
	zone->SetFogEnd(350.0f);

	// Create a directional light to the world. Enable cascaded shadows on it
	Node *lightnode = scene_->CreateChild("DirectionalLight");
	lightnode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
	Light *light = lightnode->CreateComponent<Light>();
	light->SetLightType(LIGHT_DIRECTIONAL);
	light->SetCastShadows(true);
	light->SetShadowBias(BiasParameters(0.0025f, 0.5f));
	light->SetShadowCascade(CascadeParameters(20.0f, 100.0f, 400.0f, 0.0f, 1.6f));
	light->SetSpecularIntensity(0.5f);
	light->SetColor(Color(1.1f, 1.1f, 1.0f));
	lightnode->SetDirection(Vector3(1.5,3.5,-1.5));
		
	Node *backLightNode = scene_->CreateChild();
	Light *backlight=backLightNode->CreateComponent<Light>();
	backlight->SetLightType(LIGHT_DIRECTIONAL);
	backlight->SetCastShadows(false);
	backlight->SetColor(Color(0.10f, 0.10f, 0.10f));
	backLightNode->SetDirection(Vector3(-1.5,-3.5,1.5));
	
	box_ = CreateObject("Models/Box.mdl", "Materials/White.xml");
	
	// Create the handles and position them. 
	xpos_ = CreateObject("Models/Icosphere.mdl", "Materials/Red.xml");
	xneg_ = CreateObject("Models/Icosphere.mdl", "Materials/Red.xml");
	ypos_ = CreateObject("Models/Icosphere.mdl", "Materials/Green.xml");
	yneg_ = CreateObject("Models/Icosphere.mdl", "Materials/Green.xml");
	zpos_ = CreateObject("Models/Icosphere.mdl", "Materials/Blue.xml");
	zneg_ = CreateObject("Models/Icosphere.mdl", "Materials/Blue.xml");
	
	xpos_->SetPosition(Vector3(1.5,0,0));
	xneg_->SetPosition(Vector3(-1.5,0,0));
	ypos_->SetPosition(Vector3(0,1.5,0));
	yneg_->SetPosition(Vector3(0,-1.5,0));
	zpos_->SetPosition(Vector3(0,0,1.5));
	zneg_->SetPosition(Vector3(0,0,-1.5));
	
	xpos_->SetScale(Vector3(0.1,0.1,0.1));
	xneg_->SetScale(Vector3(0.1,0.1,0.1));
	ypos_->SetScale(Vector3(0.1,0.1,0.1));
	yneg_->SetScale(Vector3(0.1,0.1,0.1));
	zpos_->SetScale(Vector3(0.1,0.1,0.1));
	zneg_->SetScale(Vector3(0.1,0.1,0.1));
}

void Game::Stop()
{
	engine_->DumpResources(true);
}
Node *Game::CreateObject(const ea::string &modelname, const ea::string &materialname)
{
	auto cache=GetSubsystem<ResourceCache>();
	
	Node *n = scene_->CreateChild();
	StaticModel *m = n->CreateComponent<StaticModel>();
	m->SetModel(cache->GetResource<Model>(modelname));
	m->SetMaterial(cache->GetResource<Material>(materialname));
	return n;
}

void Game::SetWindowTitleAndIcon()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Image* icon = cache->GetResource<Image>("Textures/UrhoIcon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("GizmoTest");
}

void Game::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
	using namespace KeyUp;
    int key = eventData[P_KEY].GetInt();
	if (key == KEY_ESCAPE)
    {
        engine_->Exit();
    }
}

void Game::MoveCamera(float timeStep)
{
	auto input = GetSubsystem<Input>();
	// Movement speed as world units per second
	const float MOVE_SPEED = 30.0f;
	// Mouse sensitivity as degrees per pixel
	const float MOUSE_SENSITIVITY = 0.5f;

	// Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
	if(input->GetMouseButtonDown(MOUSEB_RIGHT))
	{
		IntVector2 mouseMove = input->GetMouseMove();
		yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
		pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
		pitch_ = Clamp(pitch_, -90.0f, 90.0f);

		// Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
		cameranode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));
		//URHO3D_LOGINFOF("Pitch: %.2f", pitch_);
	}

	// Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
	if (input->GetKeyDown(KEY_W))
		cameranode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_S))
		cameranode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_A))
		cameranode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_D))
		cameranode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
		
}


void Game::HandleUpdate(StringHash eventType, VariantMap &eventData)
{
	float dt=eventData["TimeStep"].GetFloat();
	
	MoveCamera(dt);
	
	auto input = GetSubsystem<Input>();
	float speed=200.f;
	
	if(input->GetMouseButtonPress(MOUSEB_LEFT))
	{
		Node *p = Pick();
		if(p==xpos_) axis_=XPOS;
		else if(p==xneg_) axis_=XNEG;
		else if(p==ypos_) axis_=YPOS;
		else if(p==yneg_) axis_=YNEG;
		else if(p==zpos_) axis_=ZPOS;
		else if(p==zneg_) axis_=ZNEG;
		else axis_=NONE;
		
		if(axis_ != NONE)
		{
			lastmouse_ = input->GetMousePosition();
			lastmouseproj_ = ReverseProject(lastmouse_, 10);
		}
	}
	
	if(input->GetMouseButtonDown(MOUSEB_LEFT))
	{
		if(axis_==NONE) return;
		IntVector2 mouse = input->GetMousePosition();
		Vector3 mouseproj = ReverseProject(lastmouse_, 10);
		
		Vector3 delta=mouseproj - lastmouseproj_;
		lastmouse_ = mouse;
		lastmouseproj_ = mouseproj;
		
		if(axis_==XPOS)
		{
			float amt = delta.x_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.x_ + amt);
			float translate = newscale - scale.x_;
			scale.x_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(1,0,0) * translate*0.5f);
			xpos_->Translate(Vector3(1,0,0) * translate);
			
			// Move other gizmos
			ypos_->Translate(Vector3(1,0,0) * translate*0.5f);
			zpos_->Translate(Vector3(1,0,0) * translate*0.5f);
			yneg_->Translate(Vector3(1,0,0) * translate*0.5f);
			zneg_->Translate(Vector3(1,0,0) * translate*0.5f);
		}
		else if(axis_==XNEG)
		{
			float amt = -delta.x_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.x_ + amt);
			float translate = newscale - scale.x_;
			scale.x_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(-1,0,0) * translate*0.5f);
			xneg_->Translate(Vector3(-1,0,0) * translate);
			
			// Move other gizmos
			ypos_->Translate(Vector3(-1,0,0) * translate*0.5f);
			zpos_->Translate(Vector3(-1,0,0) * translate*0.5f);
			yneg_->Translate(Vector3(-1,0,0) * translate*0.5f);
			zneg_->Translate(Vector3(-1,0,0) * translate*0.5f);
		}
		else if(axis_==YPOS)
		{
			float amt = delta.y_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.y_ + amt);
			float translate = newscale - scale.y_;
			scale.y_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(0,1,0) * translate*0.5f);
			ypos_->Translate(Vector3(0,1,0) * translate);
			
			// Move other gizmos
			xpos_->Translate(Vector3(0,1,0) * translate*0.5f);
			zpos_->Translate(Vector3(0,1,0) * translate*0.5f);
			xneg_->Translate(Vector3(0,1,0) * translate*0.5f);
			zneg_->Translate(Vector3(0,1,0) * translate*0.5f);
		}
		else if(axis_==YNEG)
		{
			float amt = -delta.y_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.y_ + amt);
			float translate = newscale - scale.y_;
			scale.y_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(0,-1,0) * translate*0.5f);
			yneg_->Translate(Vector3(0,-1,0) * translate);
			
			xpos_->Translate(Vector3(0,-1,0) * translate*0.5f);
			zpos_->Translate(Vector3(0,-1,0) * translate*0.5f);
			xneg_->Translate(Vector3(0,-1,0) * translate*0.5f);
			zneg_->Translate(Vector3(0,-1,0) * translate*0.5f);
		}
		else if(axis_==ZPOS)
		{
			float amt = delta.z_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.z_ + amt);
			float translate = newscale - scale.z_;
			scale.z_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(0,0,1) * translate*0.5f);
			zpos_->Translate(Vector3(0,0,1) * translate);
			
			xpos_->Translate(Vector3(0,0,1) * translate*0.5f);
			ypos_->Translate(Vector3(0,0,1) * translate*0.5f);
			xneg_->Translate(Vector3(0,0,1) * translate*0.5f);
			yneg_->Translate(Vector3(0,0,1) * translate*0.5f);
		}
		else if(axis_==ZNEG)
		{
			float amt = -delta.z_;
			Vector3 scale = box_->GetScale();
			float newscale = std::max(1.f, scale.z_ + amt);
			float translate = newscale - scale.z_;
			scale.z_ = newscale;
			box_->SetScale(scale);
			box_->Translate(Vector3(0,0,-1) * translate*0.5f);
			zneg_->Translate(Vector3(0,0,-1) * translate);
			
			xpos_->Translate(Vector3(0,0,-1) * translate*0.5f);
			ypos_->Translate(Vector3(0,0,-1) * translate*0.5f);
			xneg_->Translate(Vector3(0,0,-1) * translate*0.5f);
			yneg_->Translate(Vector3(0,0,-1) * translate*0.5f);
		}
	}
}

bool Game::Raycast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable)
{
    hitDrawable = nullptr;

    auto* ui = GetSubsystem<UI>();
    IntVector2 pos = ui->GetCursorPosition();
    // Check the cursor is visible and there is no UI element in front of the cursor
    if (!ui->GetCursor()->IsVisible() || ui->GetElementAt(pos, true))
        return false;

    pos = ui->ConvertUIToSystem(pos);

    auto* graphics = GetSubsystem<Graphics>();
    auto* camera = cameranode_->GetComponent<Camera>();
    Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
    // Pick only geometry objects, not eg. zones or lights, only get the first (closest) hit
    ea::vector<RayQueryResult> results;
    RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
    scene_->GetComponent<Octree>()->RaycastSingle(query);
    if (results.size())
    {
        RayQueryResult& result = results[0];
        hitPos = result.position_;
        hitDrawable = result.drawable_;
        return true;
    }

    return false;
}

Node *Game::Pick()
{
	Drawable *d=nullptr;
	Vector3 hitpos;
	
	if(!Raycast(100, hitpos, d)) return nullptr;
	if(d) return d->GetNode();
	return nullptr;
}

Vector3 Game::ReverseProject(const IntVector2 &mpos, float z)
{
	if(!cameranode_) return Vector3();
	auto graphics = GetSubsystem<Graphics>();
	
	Camera *camera = cameranode_->GetComponent<Camera>();
	return camera->ScreenToWorldPoint(Vector3((float)mpos.x_ / (float)graphics->GetWidth(), (float)mpos.y_ / (float)graphics->GetHeight(), z));
}

URHO3D_DEFINE_APPLICATION_MAIN(Game)

