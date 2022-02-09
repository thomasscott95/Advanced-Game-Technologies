#pragma once
#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine(PushdownState* initialState){
				this->initialState = initialState;
			}
			~PushdownMachine() {}

			void SetInitialState(bool set) {
				setinitialState = set;
			}

			bool Update(float dt);

		protected:
			PushdownState * activeState;
			PushdownState* initialState;
			bool setinitialState;

			std::stack<PushdownState*> stateStack;
		};
	}
}

