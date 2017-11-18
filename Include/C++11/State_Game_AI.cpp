#include "State_Game_AI.h"
#include <LWPlatform/LWInputDevice.h>
#include <LWPlatform/LWWindow.h>
#include <LWCore/LWMath.h>
#include "App.h"

State &State_Game_AI::Update(App *A, bool Tick, uint64_t lCurrentTime) {
	if (!Tick) return *this;
	if (m_GameMenuRect->GetVisible()) return *this;
	/* Predict ball path! */
	const uint32_t MaxRaysCasted = 3;
	const uint32_t MaxSimulatedBlockers = 4;
	GameObject *Ball = m_Game.GetBall();
	LWVector2f CurrPos = Ball->GetPosition();
	LWVector2f CurrDir = Ball->GetVelocity().Normalize();

	GameObject SimulatedBlockers[MaxSimulatedBlockers];
	SimulatedBlockers[0] = GameObject(LWVector2f(m_Game.GetFieldSize().x*0.5f, -2.5f), LWVector2f(), LWVector2f(m_Game.GetFieldSize().x*0.5f, 2.5f), 0.0f, GameObject::BlockerA, GameObject::Rect);
	SimulatedBlockers[1] = GameObject(LWVector2f(m_Game.GetFieldSize().x*0.5f, 2.5f), LWVector2f(), LWVector2f(m_Game.GetFieldSize().x*0.5f, 2.5f), 0.0f, GameObject::BlockerA, GameObject::Rect);
	uint32_t SimulatedCnt = 2;
	if ((m_Tick % 15) == 0) {
		for (uint32_t i = 0; i < MaxRaysCasted; i++) {
			LWVector2f HitPnt;
			LWVector2f HitNrm;
			GameObject *Hit = ProcessRay(CurrPos, CurrDir, SimulatedBlockers, SimulatedCnt, &HitPnt, &HitNrm);
			if (!Hit) break;
			HitPnt += HitNrm*Ball->GetSize().x;
			if (Hit->GetID() == GameObject::GoalB) break;
			if (Hit->GetID() == GameObject::GoalA) {
				float T = A->Random(-LW_PI_2*0.55f, LW_PI_2*0.55f);
				if (SimulatedCnt >= MaxSimulatedBlockers) continue;
				SimulatedBlockers[SimulatedCnt++] = GameObject(CurrPos + (HitPnt - CurrPos)*0.6f, LWVector2f(), Game::BlockSize, T, GameObject::BlockerA, GameObject::Rect);
				i = -1; //Reset simulation!
				CurrPos = Ball->GetPosition();
				CurrDir = Ball->GetVelocity().Normalize();
				continue;
			}
			CurrPos = HitPnt;
			CurrDir = CurrDir - (HitNrm*CurrDir.Dot(HitNrm)*2.0f);
		}
	}
	if(SimulatedCnt>2) PushBlocker(SimulatedBlockers[2].GetPosition(), SimulatedBlockers[2].GetTheta(), GameObject::BlockerA);
	m_Tick++;
	return State_Game::Update(A, Tick, lCurrentTime);
}

State &State_Game_AI::DrawFrame(Renderer *R, Frame *F, App *A) {
	State_Game::DrawFrame(R, F, A);
	return *this;
}


GameObject *State_Game_AI::ProcessRay(const LWVector2f &Start, const LWVector2f &Dir, GameObject *SimulatedBlockers, uint32_t SimulateCnt, LWVector2f *OutPnt, LWVector2f *OutNrm) {
	GameObject *Ball = m_Game.GetBall();

	LWVector2f BallTop = Start + Ball->GetSize().x*Dir.Perpindicular();
	LWVector2f BallBtm = Start - Ball->GetSize().x*Dir.Perpindicular();
	LWVector2f TopEnd = BallTop + Dir*m_Game.GetFieldSize().x;
	LWVector2f BtmEnd = BallBtm + Dir*m_Game.GetFieldSize().x;
	LWVector2f TopPnt;
	LWVector2f TopNrm;
	LWVector2f BtmPnt;
	LWVector2f BtmNrm;

	GameObject *Top = m_Game.TestRay(BallTop, TopEnd, SimulatedBlockers, SimulateCnt, &TopPnt, &TopNrm);
	GameObject *Btm = m_Game.TestRay(BallBtm, BtmEnd, SimulatedBlockers, SimulateCnt, &BtmPnt, &BtmNrm);

	if (!Top && !Btm) return nullptr;

	float TopD = Top ? (TopEnd - BallTop).Dot((TopPnt - BallTop)) : 100000000.0f;
	float BtmD = Btm ? (BtmEnd - BallTop).Dot((BtmPnt - BallBtm)) : 100000000.0f;
	if (TopD < BtmD) {
		if (OutPnt) *OutPnt = TopPnt - Ball->GetSize().x*Dir.Perpindicular();
		if (OutNrm) *OutNrm = TopNrm;
		//std::cout << "Top: " << (*OutPnt).x << " " << (*OutPnt).y << std::endl;
		return Top;
	}else{
		if (OutPnt) *OutPnt = BtmPnt + Ball->GetSize().x*Dir.Perpindicular();
		if (OutNrm) *OutNrm = BtmNrm;
		//std::cout << "Btm: " << (*OutPnt).x << " " << (*OutPnt).y << std::endl;
		return Btm;
	}

}

State &State_Game_AI::ProcessInput(LWWindow *Wnd, App *A) {
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWEUIManager *UIMan = A->GetUIManager();
	if (Mouse) {
		if (UIMan->GetOverCount() == 0) {
			if (Mouse->ButtonPressed(LWMouseKey::Left)) {
				PushBlocker(m_Game.MapScreen(Mouse->GetPositionf(), Wnd->GetSizef()), m_Theta, m_SelectedBlocker);
			}
		}

		if (Mouse->GetScroll()) {
			if (Mouse->GetScroll() > 0.0f) m_Theta += 0.1f;
			else m_Theta -= 0.1f;
		}
	}
	return *this;
}

State_Game_AI::State_Game_AI(LWEUIManager *UIMan, App *A) : State_Game(UIMan, A), m_Tick(0){
	m_SelectedBlocker = GameObject::BlockerB;
}

