#pragma once

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Drawable.h>

using namespace Urho3D;


class Game : public Application
{
	URHO3D_OBJECT(Game, Application);

	public:
	explicit Game(Context *context);

	void Setup() override;
	void Start() override;
	void Stop() override;

	private:
	void SetWindowTitleAndIcon();
	void HandleKeyDown(StringHash eventType, VariantMap& eventData);
	void HandleUpdate(StringHash eventType, VariantMap &eventData);
	
	void MoveCamera(float dt);
	Node *CreateObject(const ea::string &modelname, const ea::string &materialname);
	Vector3 ReverseProject(const IntVector2 &mpos, float z);
	bool Raycast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable);
	Node *Pick();
	
	Node *cameranode_{nullptr}, *box_{nullptr}, *xpos_{nullptr}, *xneg_{nullptr}, *ypos_{nullptr}, *yneg_{nullptr}, *zpos_{nullptr}, *zneg_{nullptr};
	float yaw_{0}, pitch_{-20.7f};
	SharedPtr<Scene> scene_;
	
	enum AxisType
	{
		XPOS,
		XNEG,
		YPOS,
		YNEG,
		ZPOS,
		ZNEG,
		NONE
	};
	
	AxisType axis_{NONE};
	IntVector2 lastmouse_;
	Vector3 lastmouseproj_;
};
