#ifndef STATE_MENU_H
#define STATE_MENU_H
#include "State.h"
#include <LWCore/LWTypes.h>
#include "Renderer.h"

class State_Menu : public State {
public:

	State &Update(App *A, bool Tick, uint64_t lCurrentTime);

	State &DrawFrame(Renderer *R, Frame *F, App *A);

	State &ProcessInput(LWWindow *Wnd, App *A);

	State &Activate(App *A);

	State &Deactivated(App *A);
private:
};

#endif