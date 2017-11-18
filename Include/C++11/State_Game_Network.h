#ifndef STATE_GAME_NETWORK_H
#define STATE_GAME_NETWORK_H
#include "State_Game.h"

class State_Game_Network : public State_Game {
public:

	virtual State &Update(App *A, bool Tick, uint64_t lCurrentTime);

	virtual State &DrawFrame(Renderer *R, Frame *F, App *A);

	virtual State &ProcessInput(LWWindow *Wnd, App *A);

	State_Game_Network(LWEUIManager *UIMan, App *A);
};

#endif