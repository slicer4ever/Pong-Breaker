#include "State_Game_Network.h"

State &State_Game_Network::Update(App *A, bool Tick, uint64_t lCurrentTime) {
	if (!Tick) return *this;

	return State_Game::Update(A, Tick, lCurrentTime);
}

State &State_Game_Network::DrawFrame(Renderer *R, Frame *F, App *A) {
	return State_Game::DrawFrame(R, F, A);
}

State &State_Game_Network::ProcessInput(LWWindow *Wnd, App *A) {
	return *this;
}

State_Game_Network::State_Game_Network(LWEUIManager *UIMan, App *A) : State_Game(UIMan, A){
}