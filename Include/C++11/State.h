#ifndef STATE_H
#define STATE_H
#include "Types.h"
#include <LWCore/LWTypes.h>
#include <LWPlatform/LWTypes.h>
#include "Renderer.h"

class State {
public:
	enum {
		Menu,
		GameS,
		GameAI,
		GameNetwork,
		Count
	};

	virtual State &Update(App *A, bool Tick, uint64_t lCurrentTime) = 0;

	virtual State &DrawFrame(Renderer *R, Frame *F, App *A) = 0;

	virtual State &ProcessInput(LWWindow *Wnd, App *A) = 0;

	virtual State &Activate(App *A) = 0;

	virtual State &Deactivated(App *A) = 0;
private:
};

#endif
