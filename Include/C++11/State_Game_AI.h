#ifndef STATE_GAME_AI_H
#define STATE_GAME_AI_H
#include "State_Game.h"
#include "Game.h"

class State_Game_AI : public State_Game {
public:

	State &Update(App *A, bool Tick, uint64_t lCurrentTime);

	GameObject *ProcessRay(const LWVector2f &Start, const LWVector2f &Dir, GameObject *SimulatedBlockers, uint32_t SimulateCnt, LWVector2f *OutPnt, LWVector2f *OutNrm);

	State &DrawFrame(Renderer *R, Frame *F, App *A);

	State &ProcessInput(LWWindow *Wnd, App *A);

	State_Game_AI(LWEUIManager *UIMan, App *A);
protected:
	uint32_t m_Tick;
};

#endif