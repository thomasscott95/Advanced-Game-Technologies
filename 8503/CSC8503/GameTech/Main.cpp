#include "../../Common/Window.h"

#include "../CSC8503Common/StateMachine.h"


#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "TutorialGame2.h"

#include "../CSC8503Common/PushdownMachine.h"
#include "../CSC8503Common/PushdownState.h"

#include"GameTechRenderer.h"




using namespace NCL;
using namespace CSC8503;

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;

	State* A = new State([&](float dt)->void {
		std::cout << "I’m in state A!\n";
		data++;
		}
	);

	State* B = new State([&](float dt)->void {
		std::cout << "I’m in state B!\n";
		data--;
		}
	);

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool
		{
			return data > 10;
		}
	);
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool
		{
			return data < 0;
		}
	);
	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);

	for (int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);

	}
}

class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,
		PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
			return PushdownResult::Pop;

		}
		return PushdownResult::NoChange;
	}
	void OnAwake() override {
		std::cout << "Press U to unpause game!\n";
	}
};

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt,
		PushdownState** newState) override {
		pauseReminder -= dt;
		if (pauseReminder < 0) {
			//std::cout << "Coins mined: " << coinsMined << "\n";
			//std::cout << "Press P to pause game , or F1 to return to main menu!\n";
			pauseReminder += 1.0f;

		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
			*newState = new PauseScreen();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1)) {
			//std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		if (rand() % 7 == 0) {
			coinsMined++;

		}
		return PushdownResult::NoChange;
	};

	void OnAwake() override {
		//std::cout << "Preparing to mine coins!\n";
	}
protected:
	int coinsMined = 0;
	float pauseReminder = 1;
};

class Level2 : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};

	void Loadlevel2() {
		Window* gamewindow = Window::GetWindow();
		srand(time(0));
		gamewindow->ShowOSPointer(false);
		gamewindow->LockMouseToWindow(true);

		TutorialGame2* g = new TutorialGame2();
		gamewindow->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
		//TestPathfinding();

		while (gamewindow->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
			float dt = gamewindow->GetTimer()->GetTimeDeltaSeconds();
			if (dt > 0.1f) {
				std::cout << "Skipping large time delta" << std::endl;
				continue; //must have hit a breakpoint or something to have a 1 second frame time!
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
				gamewindow->ShowConsole(true);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
				gamewindow->ShowConsole(false);
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
				gamewindow->SetWindowPosition(0, 0);
			}

			gamewindow->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

			//DisplayPathfinding();

			g->UpdateGame(dt);
		}
		PushdownResult::Pop;

	}
	void OnAwake() override {
		std::cout << "Game Loading!\n";
		Loadlevel2();
	}
	void OnSleep() override {
		std::cout << "Going to Sleep!\n";

	}
};

class Level1 : public PushdownState  {
	PushdownResult OnUpdate(float dt,PushdownState* *newState) override {
		
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};

	void Loadlevel1() {
		Window* gamewindow = Window::GetWindow();
		srand(time(0));
		gamewindow->ShowOSPointer(false);
		gamewindow->LockMouseToWindow(true);

		TutorialGame* g = new TutorialGame();
		gamewindow->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
		//TestPathfinding();

		while (gamewindow->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
			float dt = gamewindow->GetTimer()->GetTimeDeltaSeconds();
			if (dt > 0.1f) {
				std::cout << "Skipping large time delta" << std::endl;
				continue; //must have hit a breakpoint or something to have a 1 second frame time!
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
				gamewindow->ShowConsole(true);
			}
			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
				gamewindow->ShowConsole(false);
			}

			if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
				gamewindow->SetWindowPosition(0, 0);
			}

			gamewindow->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

			//DisplayPathfinding();

			g->UpdateGame(dt);
		}
	}
	void OnAwake() override {
		std::cout << "Game Loading!\n";
		Loadlevel1();
	}
	void OnSleep() override {
		std::cout << "Going to Sleep!\n";
		
	}
};

class IntroScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState * *newState) override {
		Debug::FlushRenderables(dt);
		renderer->Render();
		renderer->DrawString("Press 1 for level 1",Vector2(10,30),Vector4(1,0,0,0),20);
		renderer->DrawString("Press 2 for level 2", Vector2(60, 30), Vector4(0, 1, 0, 0), 20);
		renderer->DrawString("Press Escape to quit", Vector2(30, 50), Vector4(0, 0, 1, 0), 20);
		
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
			*newState = new Level1();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
			*newState = new Level2();
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Quit;
		}
		return PushdownResult::NoChange;
};

	void OnAwake() override {
		world = new GameWorld();
		renderer = new GameTechRenderer(*world);
		Debug::SetRenderer(renderer);
		
		std::cout << "Welcome to a really awesome game!\n";
		//std::cout << "Press Space To Begin or escape to quit!\n";
	}

protected:
	GameWorld* world;
	GameTechRenderer* renderer;
};


IntroScreen mainMenu;

void TestPushdownAutomata(Window* w) {
	PushdownMachine machine(&mainMenu);
	while (w->UpdateWindow()) {
	float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
	}
}


int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	TestPushdownAutomata(w);


	Window::DestroyGameWindow();

}