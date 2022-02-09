#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

#include "PhysicsObject.h"
#include "RenderObject.h"

#include <vector>

using std::vector;

namespace NCL {
	namespace CSC8503 {

		class GameObject	{
		public:
			GameObject(string name = "");
			~GameObject();

			void SetBoundingVolume(CollisionVolume* vol) {
				boundingVolume = vol;
			}

			const CollisionVolume* GetBoundingVolume() const {
				return boundingVolume;
			}

			bool IsActive() const {
				return isActive;
			}

			void setActive(bool name) {
				isActive = name;
			}

			Transform& GetTransform() {
				return transform;
			}

			RenderObject* GetRenderObject() const {
				return renderObject;
			}

			PhysicsObject* GetPhysicsObject() const {
				return physicsObject;
			}



			void SetRenderObject(RenderObject* newObject) {
				renderObject = newObject;
			}

			void SetPhysicsObject(PhysicsObject* newObject) {
				physicsObject = newObject;
			}

			const string& GetName() const {
				return name;
			}
			bool HasWon() {
				return hasWon;
			}
			bool HasLost() {
				return hasLost;
			}
			bool HasCollected() {
				return hasCollected;
			}
			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
				if (this->GetName() == "Player" && otherObject->GetName() == "FinishLine") {
					hasWon = true;
				}

				if (this->GetName() == "Player" && otherObject->GetName() == "Lava") {
					hasLost = true;
				}
				if (this->GetName() == "Player" && otherObject->GetName() == "Bonus") {
					hasCollected = true;
				}
				
			}


				
			
			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

			bool GetBroadphaseAABB(Vector3&outsize) const;

			void UpdateBroadphaseAABB();

			void SetWorldID(int newID) {
				worldID = newID;
			}

			int		GetWorldID() const {
				return worldID;
			}

		protected:
			Transform			transform;

			CollisionVolume*	boundingVolume;
			PhysicsObject*		physicsObject;
			RenderObject*		renderObject;

			bool	isActive;
			int		worldID;
			string	name;
			bool hasWon = false;
			bool hasLost = false;
			bool hasCollected = false;

			Vector3 broadphaseAABB;
		};
	}
}

