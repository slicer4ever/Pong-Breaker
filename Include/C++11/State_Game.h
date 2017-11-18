#ifndef STATE_GAME_H
#define STATE_GAME_H
#include "State.h"
#include "Game.h"

struct BlockerSpawns {
	LWVector2f m_Position;
	uint32_t m_Blocker;
	float m_Theta;
};

class State_Game : public State {
public:
	enum {
		MaxBlockerQueue = 8
	};

	virtual State &Update(App *A, bool Tick, uint64_t lCurrentTime);

	virtual State &DrawFrame(Renderer *R, Frame *F, App *A);

	virtual State &ProcessInput(LWWindow *Wnd, App *A);

	bool PushBlocker(const LWVector2f &Position, float Theta, uint32_t BlockerID);

	void GameMenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void GameMenuBackBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	void GameMenuMenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData);

	State &Activate(App *A);

	State &Deactivated(App *A);

	State_Game(LWEUIManager *UIMan, App *A);

protected:
	Game m_Game;
	float m_Theta;
	BlockerSpawns m_Blockers[MaxBlockerQueue];
	uint32_t m_BlockerCnt;
	uint32_t m_SelectedBlocker;

	LWEUI *m_GameRect;
	LWEUI *m_GameMenuRect;
	LWEUILabel *m_ScoreLabel;
};

#endif