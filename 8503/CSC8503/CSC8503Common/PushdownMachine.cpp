#include "PushdownMachine.h"
#include "PushdownState.h"
using namespace NCL::CSC8503;


bool PushdownMachine::Update(float dt) {
	if (activeState) {
		PushdownState* newState = nullptr;
		PushdownState::PushdownResult result = activeState->OnUpdate(dt, &newState);

		switch (result) {
			case PushdownState::Pop: {
				activeState->OnSleep();
				delete activeState;
				stateStack.pop();
				
				if (stateStack.empty()) {
					return false;
				}
				
				else {
					activeState = stateStack.top();
					activeState->OnAwake();
				}

			}break;

			case PushdownState::Push: {
				activeState->OnSleep();
				stateStack.push(activeState);
				activeState = newState;
				activeState->OnAwake();
			}break;

			case PushdownState::Quit: {
					exit(0);
				}
		}
	}
	else {
		stateStack.push(initialState);
		activeState = initialState;
		activeState->OnAwake();
	}
	return true;
}