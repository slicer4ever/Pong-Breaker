#include "State_Menu.h"

State &State_Menu::Update(App *A, bool Tick, uint64_t lCurrentTime) {
	return *this;
}

State &State_Menu::DrawFrame(Renderer *R, Frame *F, App *A) {
	return *this;
}

State &State_Menu::ProcessInput(LWWindow *Wnd, App *A) {
	return *this;
}

State &State_Menu::Activate(App *A) {
	return *this;
}

State &State_Menu::Deactivated(App *A) {
	return *this;
}