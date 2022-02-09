#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "../../Common/TextureLoader.h"
#include "../CSC8503Common/PositionConstraint.h"




using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer	= new GameTechRenderer(*world);
	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= false;
	inSelectionMode = false;

	Debug::SetRenderer(renderer);

	InitialiseAssets();
}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, OGLMesh** into) {
		*into = new OGLMesh(name);
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};

	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);

	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");


	blue = (OGLTexture*)TextureLoader::LoadAPITexture("blue.png");
	ice = (OGLTexture*)TextureLoader::LoadAPITexture("ice.png");
	finishline = (OGLTexture*)TextureLoader::LoadAPITexture("finishline.png");
	lava = (OGLTexture*)TextureLoader::LoadAPITexture("lava.png");
	green = (OGLTexture*)TextureLoader::LoadAPITexture("green.png");
	purple = (OGLTexture*)TextureLoader::LoadAPITexture("purple.png");
	red = (OGLTexture*)TextureLoader::LoadAPITexture("red.png");

	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	SelectObject();
	MoveSelectedObject();
	physics->Update(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}
	if (player)if (player->HasWon()) hasWon = player->HasWon();
	if (player)if (player->HasLost()) hasLost = player->HasLost();
	if (player)if (player->HasCollected()) hasCollected = player->HasCollected();
	EndScreen();
	GameLostScreen();
	ItemCollected();
	world->UpdateWorld(dt);
	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
	
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		Vector3 worldPos = selectionObject->GetTransform().GetPosition();
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * force);
	}

	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
		lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	}
}

void TutorialGame::DebugObjectMovement() {
	if (inSelectionMode && selectionObject) {
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::M) && selectionObject->GetName() == "Player") {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 100, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A) && selectionObject->GetName() == "Bridge") {
			selectionObject->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 1));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D) && selectionObject->GetName() == "Bridge") {
			selectionObject->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, -1));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT) && selectionObject->GetName() == "Push") {
			selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, -10));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT) && selectionObject->GetName() == "Push") {
			selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN) && selectionObject->GetName() == "RotatePush") {
			selectionObject->GetPhysicsObject()->SetAngularVelocity(Vector3(-6, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN) && selectionObject->GetName() == "RotatePush2") {
			selectionObject->GetPhysicsObject()->SetAngularVelocity(Vector3(-4, 0, 0));
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP) && selectionObject->GetName() == "Catch") {
			selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, -6));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN) && selectionObject->GetName() == "Catch") {
			selectionObject->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 6));
		}
	}
//If we've selected an object, we can manipulate it with some key presses
	else if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}


}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();
	InitDefaultFloor(); // if ball touches this floor will respawn back at start.
	InitGameWalls();
	InitSpherePlayer();
	InitFloorPanel();
	InitSpinner();
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(1.5, 1.5, 1.5);

	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 2;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3(-150, 0, -150);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, -10, -0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;

	}
	PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	world->AddConstraint(constraint);

}

void TutorialGame::BigSwing() {
	Vector3 cubeSize = Vector3(4, 4, 4);

	float invCubeMass = 5; //how heavy the middle pieces are
	int numLinks = 4;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links

	Vector3 startPos = Vector3 (87.5, 120, -87.5);

	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	//GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);

	GameObject* previous = start;

	for (int i = 0; i < numLinks; ++i) {
		GameObject* block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint* constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;

	}
	//PositionConstraint* constraint = new PositionConstraint(previous, end, maxDistance);
	//world->AddConstraint(constraint);

}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position) {
	GameObject* floor = new GameObject();

	Vector3 floorSize	= Vector3(100, 2, 100);
	OBBVolume* volume	= new OBBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

GameObject* TutorialGame::AddWalls(const Vector3& position, Vector3 dimensions, float inverseMass, string name) {
	GameObject* walls = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	walls->SetBoundingVolume((CollisionVolume*)volume);

	walls->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	walls->SetRenderObject(new RenderObject(&walls->GetTransform(), cubeMesh, basicTex, basicShader));
	walls->SetPhysicsObject(new PhysicsObject(&walls->GetTransform(), walls->GetBoundingVolume()));

	walls->GetPhysicsObject()->SetInverseMass(inverseMass);
	walls->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(walls);

	return walls;
}

GameObject* TutorialGame::AddFloorPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* panel = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	panel->SetBoundingVolume((CollisionVolume*)volume);

	panel->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	panel->SetRenderObject(new RenderObject(&panel->GetTransform(), cubeMesh, basicTex, basicShader));
	panel->SetPhysicsObject(new PhysicsObject(&panel->GetTransform(), panel->GetBoundingVolume()));

	panel->GetPhysicsObject()->SetInverseMass(0);
	panel->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(panel);

	return panel;
}

GameObject* TutorialGame::AddRotPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* rot = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	rot->SetBoundingVolume((CollisionVolume*)volume);

	rot->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	rot->SetRenderObject(new RenderObject(&rot->GetTransform(), cubeMesh, lava, basicShader));
	rot->SetPhysicsObject(new PhysicsObject(&rot->GetTransform(), rot->GetBoundingVolume()));

	rot->GetPhysicsObject()->SetInverseMass(0);
	rot->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(rot);

	return rot;
}

GameObject* TutorialGame::AddPushFloorPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* push = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	push->SetBoundingVolume((CollisionVolume*)volume);

	push->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	push->SetRenderObject(new RenderObject(&push->GetTransform(), cubeMesh, green, basicShader));
	push->SetPhysicsObject(new PhysicsObject(&push->GetTransform(), push->GetBoundingVolume()));

	push->GetPhysicsObject()->SetInverseMass(0);
	push->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(push);

	return push;
}

GameObject* TutorialGame::AddCatchPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* ctch = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	ctch->SetBoundingVolume((CollisionVolume*)volume);

	ctch->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	ctch->SetRenderObject(new RenderObject(&ctch->GetTransform(), cubeMesh, red, basicShader));
	ctch->SetPhysicsObject(new PhysicsObject(&ctch->GetTransform(), ctch->GetBoundingVolume()));

	ctch->GetPhysicsObject()->SetInverseMass(0);
	ctch->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(ctch);

	return ctch;
}

GameObject* TutorialGame::AddFloorPanelLava(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* lavaFloor = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	lavaFloor->SetBoundingVolume((CollisionVolume*)volume);

	lavaFloor->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	lavaFloor->SetRenderObject(new RenderObject(&lavaFloor->GetTransform(), cubeMesh, basicTex, basicShader));
	lavaFloor->SetPhysicsObject(new PhysicsObject(&lavaFloor->GetTransform(), lavaFloor->GetBoundingVolume()));

	lavaFloor->GetPhysicsObject()->SetInverseMass(0);
	lavaFloor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(lavaFloor);

	return lavaFloor;
}

GameObject* TutorialGame::AddFinishPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* finish = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	finish->SetBoundingVolume((CollisionVolume*)volume);

	finish->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	finish->SetRenderObject(new RenderObject(&finish->GetTransform(), cubeMesh, finishline, basicShader));
	finish->SetPhysicsObject(new PhysicsObject(&finish->GetTransform(), finish->GetBoundingVolume()));

	finish->GetPhysicsObject()->SetInverseMass(0);
	finish->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(finish);

	return finish;
}

GameObject* TutorialGame::AddRotBrid(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* bridge = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	bridge->SetBoundingVolume((CollisionVolume*)volume);

	bridge->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	bridge->SetRenderObject(new RenderObject(&bridge->GetTransform(), cubeMesh, purple, basicShader));
	bridge->SetPhysicsObject(new PhysicsObject(&bridge->GetTransform(), bridge->GetBoundingVolume()));

	bridge->GetPhysicsObject()->SetInverseMass(0);
	bridge->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(bridge);

	return bridge;
}

GameObject* TutorialGame::AddIceFloorPanel(const Vector3& position, Vector3 dimensions, string name) {
	GameObject* panel = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	panel->SetBoundingVolume((CollisionVolume*)volume);

	panel->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	panel->SetRenderObject(new RenderObject(&panel->GetTransform(), cubeMesh, ice, basicShader));
	panel->SetPhysicsObject(new PhysicsObject(&panel->GetTransform(), panel->GetBoundingVolume()));

	panel->GetPhysicsObject()->SetInverseMass(0);
	panel->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(panel);

	return panel;
}

GameObject* TutorialGame::AddSpinners(const Vector3& position, Vector3 dimensions, float inverseMass, string name) {
	GameObject* spinner = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	spinner->SetBoundingVolume((CollisionVolume*)volume);

	spinner->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	spinner->SetRenderObject(new RenderObject(&spinner->GetTransform(), cubeMesh, basicTex, basicShader));
	spinner->SetPhysicsObject(new PhysicsObject(&spinner->GetTransform(), spinner->GetBoundingVolume()));

	spinner->GetPhysicsObject()->SetInverseMass(inverseMass);
	spinner->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(spinner);
	spinner->GetPhysicsObject()->ApplyAngularImpulse(Vector3(0, 6, 0));

	return spinner;
}

GameObject* TutorialGame::AddSpinners2(const Vector3& position, Vector3 dimensions, float inverseMass, string name) {
	GameObject* spinner2 = new GameObject(name);

	OBBVolume* volume = new OBBVolume(dimensions);

	spinner2->SetBoundingVolume((CollisionVolume*)volume);

	spinner2->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	spinner2->SetRenderObject(new RenderObject(&spinner2->GetTransform(), cubeMesh, basicTex, basicShader));
	spinner2->SetPhysicsObject(new PhysicsObject(&spinner2->GetTransform(), spinner2->GetBoundingVolume()));

	spinner2->GetPhysicsObject()->SetInverseMass(inverseMass);
	spinner2->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(spinner2);
	spinner2->GetPhysicsObject()->ApplyAngularImpulse(Vector3(0, -6, 0));

	return spinner2;
}

GameObject* TutorialGame::AddSpherePlayer(const Vector3& position, float radius, float inverseMass, string name) {
	GameObject* spherePlayer = new GameObject(name);


	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	spherePlayer->SetBoundingVolume((CollisionVolume*)volume);

	spherePlayer->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	spherePlayer->SetRenderObject(new RenderObject(&spherePlayer->GetTransform(), sphereMesh, basicTex, basicShader));
	spherePlayer->SetPhysicsObject(new PhysicsObject(&spherePlayer->GetTransform(), spherePlayer->GetBoundingVolume()));

	spherePlayer->GetPhysicsObject()->SetInverseMass(inverseMass);
	spherePlayer->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(spherePlayer);	

	return spherePlayer;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass) {
	GameObject* sphere = new GameObject();

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject();

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;

}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass) {
	GameObject* cube = new GameObject();

	AABBVolume* volume = new AABBVolume(dimensions);

	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 10, 0), 0));

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -50, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddEnemyToWorld(Vector3(5, 5, 0));
	AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitFloorPanel() {
	AddFloorPanel(Vector3(87.5, 0, -87.5), Vector3(12.5, 2, 12.5),"Floor")->GetPhysicsObject()->SetElasticity(0); //
	AddFloorPanel(Vector3(62.5, 0, -87.5), Vector3(12.5, 2, 12.5), "Speed");

	AddFloorPanel(Vector3(38.5, 6, -87.5), Vector3(12.5, 1, 12.5), "Floor")->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, -5), 5));//

	AddFloorPanel(Vector3(-62.5, 0, -87.5), Vector3(12.5, 2, 12.5), "Floor");//
	AddFloorPanel(Vector3(-87.5, 0, -87.5), Vector3(12.5, 2, 12.5), "Floor");// First Section

	AddFloorPanel(Vector3(87.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Floor"); //
	AddIceFloorPanel(Vector3(62.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice2"); //
	AddIceFloorPanel(Vector3(37.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice3");//
	AddIceFloorPanel(Vector3(12.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice1");//
	AddIceFloorPanel(Vector3(-12.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice2");//
	AddIceFloorPanel(Vector3(-37.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice3");//
	AddIceFloorPanel(Vector3(-62.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice1");//
	AddIceFloorPanel(Vector3(-87.5, 0, -62.5), Vector3(12.5, 2, 12.5), "Ice2");// Second Section

	AddFloorPanel(Vector3(87.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1"); //
	AddFloorPanel(Vector3(62.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1"); //
	AddFloorPanel(Vector3(37.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1");//
	AddFloorPanel(Vector3(12.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1");//
	AddFloorPanel(Vector3(-12.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1");//
	AddFloorPanel(Vector3(-37.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1");//
	AddFloorPanel(Vector3(-62.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Speed1");//
	AddFloorPanel(Vector3(-87.5, 0, -37.5), Vector3(12.5, 2, 12.5), "Floor");// third Section

	AddFloorPanel(Vector3(87.5,  0, -12.5), Vector3(12.5, 2, 12.5), "Floor")->GetPhysicsObject()->SetElasticity(0); //
	AddRotBrid(Vector3(62.5,  0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0); //
	AddRotBrid(Vector3(37.5,  0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0);//
	AddRotBrid(Vector3(12.5,  0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0);//
	AddRotBrid(Vector3(-12.5, 0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0);//
	AddRotBrid(Vector3(-37.5, 0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0);//
	AddRotBrid(Vector3(-62.5, 0, -12.5), Vector3(12.5, 2, 12.5), "Bridge")->GetPhysicsObject()->SetElasticity(0);//
	AddFloorPanel(Vector3(-87.5, 0, -12.5), Vector3(12.5, 2, 12.5), "Speed2");// fourth Section

	AddFloorPanel(Vector3(87.5,  0, 12.5), Vector3(12.5, 2, 12.5), "Speed"); //
	AddFloorPanel(Vector3(62.5,  0, 12.5), Vector3(12.5, 2, 12.5), "Floor"); //
	AddFloorPanel(Vector3(38.5,  6, 12.5), Vector3(12.5, 1, 12.5), "Floor")->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, -5), 5));//

	GameObject* panel = AddFloorPanel(Vector3(-87.5, 0, 12.5), Vector3(12.5, 1, 12.5), "Floor");
	panel->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 5), 5));
	panel->GetPhysicsObject()->SetElasticity(2);// fith Section

	AddFloorPanel(Vector3(-179, 1, 12.5), Vector3(30, 1, 30), "Floor")->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, 5), 5));;

	AddFloorPanel(Vector3(-176.5, 10, -17.5), Vector3(30, 20, 0.5), "Wall")->GetPhysicsObject()->SetElasticity(1.5);
	AddFloorPanel(Vector3(-176.5, 10, 43), Vector3(30, 20, 0.5), "Wall")->GetPhysicsObject()->SetElasticity(1.5);

	AddFinishPanel(Vector3(-207.5, 5, -10), Vector3(0.5, 20, 7.5), "FinishLine")->GetPhysicsObject()->SetElasticity(0);
	AddFinishPanel(Vector3(-207.5, 5, 5), Vector3(0.5, 20, 7.5), "FinishLine")->GetPhysicsObject()->SetElasticity(0);
	AddFinishPanel(Vector3(-207.5, 5, 20), Vector3(0.5, 20, 7.5), "FinishLine")->GetPhysicsObject()->SetElasticity(0);
	AddFinishPanel(Vector3(-207.5, 5, 35), Vector3(0.5, 20, 7.5), "FinishLine")->GetPhysicsObject()->SetElasticity(0);
	AddFloorPanelLava(Vector3(0, -10, 0), Vector3(150, 0, 150), "Lava")->GetPhysicsObject()->SetElasticity(0);

	bonus = AddSpherePlayer(Vector3(-205, -8, -10), 1, 0, "Bonus");
	bonus = AddSpherePlayer(Vector3(-205, -8, 5), 1, 0, "Bonus");
	bonus = AddSpherePlayer(Vector3(-205, -8, 20), 1, 0, "Bonus");
	bonus = AddSpherePlayer(Vector3(-205, -8, 35), 1, 0, "Bonus");


	AddFloorPanel(Vector3(-75.5, 0, -31.5), Vector3(0.5, 5, 7), "Wall")->GetPhysicsObject()->SetElasticity(50); // 3rd section wall
	AddFloorPanel(Vector3(-45.5, 0, -42.5), Vector3(0.5, 5, 7), "Wall")->GetPhysicsObject()->SetElasticity(50); // outside wall
	AddFloorPanel(Vector3(-12.5, 0, -31.5), Vector3(0.5, 5, 7), "Wall")->GetPhysicsObject()->SetElasticity(50); // outside wall
	AddFloorPanel(Vector3(12.5, 0, -42.5), Vector3(0.5, 5, 7), "Wall")->GetPhysicsObject()->SetElasticity(50); // outside wall
	AddFloorPanel(Vector3(45.5, 0, -31.5), Vector3(0.5, 5, 7), "Wall")->GetPhysicsObject()->SetElasticity(50); // outside wall

	AddPushFloorPanel(Vector3(53, 5, -28.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // left
	AddPushFloorPanel(Vector3(-5, 5, -28.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // left
	AddPushFloorPanel(Vector3(-68, 5, -28.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // left

	AddPushFloorPanel(Vector3(20, 5, -47.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // right
	AddPushFloorPanel(Vector3(-38, 5, -47.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // right
	AddPushFloorPanel(Vector3(-87.5, 5, -47.5), Vector3(7, 5, 0.5), "Push")->GetPhysicsObject()->SetElasticity(1); // right

	AddFloorPanel(Vector3(-100, 5, -38.5), Vector3(0.5, 5, 7), "BackWall")->GetPhysicsObject()->SetElasticity(2); // right
	
	AddFloorPanel(Vector3(-100, 20, -87.5), Vector3(0.5, 5, 7), "BackWall")->GetPhysicsObject()->SetElasticity(2); // right
	AddFloorPanel(Vector3(-60, 5, -87.5), Vector3(0.5, 10, 7), "BackWall")->GetPhysicsObject()->SetElasticity(1); // right
	AddFloorPanel(Vector3(-100, 10, -87.5), Vector3(0.5, 5, 7), "BackWall")->GetPhysicsObject()->SetElasticity(1); // right



	AddCatchPanel(Vector3(100, 0, -62.5), Vector3(0.5, 5, 7), "Catch")->GetPhysicsObject()->SetElasticity(0.5); // outside wall
	AddFloorPanel(Vector3(100, 0, -12.5), Vector3(0.5, 5, 12.5), "bounce")->GetPhysicsObject()->SetElasticity(0.5); // outside wall


	AddRotPanel(Vector3(-90, 10, -87.5), Vector3(2, 0.5, 8), "RotatePush")->GetPhysicsObject()->SetElasticity(1.5); // right
	AddRotPanel(Vector3(90, 10, -12.5), Vector3(6, 0.5, 12.5), "RotatePush2")->GetPhysicsObject()->SetElasticity(1.5); // right
	AddRotPanel(Vector3(90, 10, -62.5), Vector3(6, 0.5, 12.5), "RotatePush2")->GetPhysicsObject()->SetElasticity(1.5); // right

	
	 
}

void TutorialGame::InitGameWalls() {
	AddWalls(Vector3(-12.5, 3, -50), Vector3(87.5, 5, 1), 0, "Wall"); // inside wall close to start 
	
	AddWalls(Vector3(12.5, 3, -25), Vector3(87.5, 5, 1), 0, "Wall");// inside wall middle 

}

void TutorialGame::InitSpinner() {
	BigSwing();
	
	
	GameObject* spinner = AddSpinners(Vector3(-179, 1, -4.5), Vector3(7, 1, 0.5), 0, "spinner");
	spinner->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, -10), 5));

	GameObject* spinner2 = AddSpinners2(Vector3(-179, 1, 12.5), Vector3(7, 1, 0.5), 0, "spinner");
	spinner2->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, -10), 5));

	GameObject* spinner3 = AddSpinners(Vector3(-179, 1, 27.5), Vector3(7, 1, 0.5), 0, "spinner");
	spinner3->GetTransform().SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 0, -10), 5));

}

void TutorialGame::InitSpherePlayer() {
	player = AddSpherePlayer(Vector3(87.5, 10, -87.5), 1, 1, "Player"); // Start of Game
	//player = AddSpherePlayer(Vector3(-87.5, 5, -62.5), 1, 1, "Player"); // Second Section of game
	//player = AddSpherePlayer(Vector3(87.5, 5, -37.5), 1, 1, "Player"); // 3rd Section of game
	//player = AddSpherePlayer(Vector3(-87.5, 5, -12.5), 1, 1, "Player"); //4th section of game
	//player = AddSpherePlayer(Vector3(87.5, 5, 12.5), 1, 1, "Player"); //5th section of game.

	

}
void TutorialGame::EndScreen() {
	if (hasWon) {
		renderer->DrawString("Won Game", Vector2(40, 40), Vector4(0, 0, 0, 0), 20);
		renderer->DrawString("Press Escape To Return To Menu", Vector2(20, 60), Vector4(0, 0, 0, 0), 20);
	}
}
void TutorialGame::GameLostScreen() {
	if (hasLost) {
		renderer->DrawString("You Lose", Vector2(40, 40), Vector4(0, 0, 0, 0), 20);
		renderer->DrawString("Press Escape To Return To Menu", Vector2(20, 60), Vector4(0, 0, 0, 0), 20);
	}
}

void TutorialGame::ItemCollected() {
	if (hasCollected) {
		renderer->DrawString("Bonus Collected", Vector2(10, 50), Vector4(0, 0, 0, 0), 20);
	}
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject();

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position) {
	StateGameObject* apple = new StateGameObject();

	SphereVolume* volume = new SphereVolume(0.25f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(0.25, 0.25, 0.25))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				Debug::DrawLine(world->GetMainCamera()->GetPosition(), closestCollision.collidedAt,Debug::CYAN,15.0f);
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject() {
	renderer->DrawString("Click Force:" + std::to_string(forceMagnitude),
		 Vector2(10, 20)); //Draw debug text at 10,20
	 forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;
	
	 if (!selectionObject) {
		 return;//we haven’t selected anything!
	 }
	 //Push the selected object!
	 if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		  Ray ray = CollisionDetection::BuildRayFromMouse(* world->GetMainCamera());
		  RayCollision closestCollision;
		  if (world->Raycast(ray, closestCollision, true)) {
			  if (closestCollision.node == selectionObject) {
				  selectionObject->GetPhysicsObject()-> AddForce(ray.GetDirection() * forceMagnitude);
			  }
			 
		  }
		 
	 }
}