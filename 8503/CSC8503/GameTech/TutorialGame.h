#pragma once
#include "GameTechRenderer.h"
#include "../CSC8503Common/PhysicsSystem.h"
#include "StateGameObject.h"



namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();

			void InitWorld();

			void InitGameExamples();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);
			void InitDefaultFloor();
			void BridgeConstraintTest();
			void BigSwing();
			void InitGameWalls();
			void InitSpherePlayer();
			void InitFloorPanel();
			void InitSpinner();
	
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();
			void EndScreen();
			void GameLostScreen();
			void ItemCollected();


			GameObject* AddFloorToWorld(const Vector3& position);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f);
			GameObject* AddWalls(const Vector3& position, Vector3 dimensions, float inverseMass, string name);
			GameObject* AddSpherePlayer(const Vector3& position, float radius, float inverseMass, string name);
			GameObject* AddFloorPanel(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddFinishPanel(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddIceFloorPanel(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddFloorPanelLava(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddSpinners(const Vector3& position, Vector3 dimensions, float inverseMass, string name);
			GameObject* AddSpinners2(const Vector3& position, Vector3 dimensions, float inverseMass, string name);
			GameObject* AddRotBrid(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddCatchPanel(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddPushFloorPanel(const Vector3& position, Vector3 dimensions, string name);
			GameObject* AddRotPanel(const Vector3& position, Vector3 dimensions, string name);
			


			GameObject* AddCapsuleToWorld(const Vector3& position, float halfHeight, float radius, float inverseMass = 10.0f);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			StateGameObject* AddStateObjectToWorld(const Vector3& position);
			StateGameObject* testStateObject;

			GameTechRenderer*	renderer;
			PhysicsSystem*		physics;
			GameWorld*			world;


			bool useGravity;
			bool inSelectionMode;
			bool hasWon = false;
			bool hasLost = false;
			bool hasCollected = false;


			float		forceMagnitude;

			GameObject* selectionObject = nullptr;

			OGLMesh*	capsuleMesh = nullptr;
			OGLMesh*	cubeMesh	= nullptr;
			OGLMesh*	sphereMesh	= nullptr;
			OGLTexture* basicTex	= nullptr;
			OGLShader*	basicShader = nullptr;

			OGLTexture* blue = nullptr;
			OGLTexture* ice = nullptr;
			OGLTexture* finishline = nullptr;
			OGLTexture* lava = nullptr;
			OGLTexture* green = nullptr;
			OGLTexture* purple = nullptr;
			OGLTexture* red = nullptr;


			//Coursework Meshes
			OGLMesh*	charMeshA	= nullptr;
			OGLMesh*	charMeshB	= nullptr;
			OGLMesh*	enemyMesh	= nullptr;
			OGLMesh*	bonusMesh	= nullptr;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;

			GameObject* player = nullptr;
			GameObject* bonus = nullptr;
			Vector3 lockedOffset		= Vector3(0, 14, 20);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

		};



	}
}

