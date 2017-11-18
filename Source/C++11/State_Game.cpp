#include "State_Game.h"
#include <LWPlatform/LWInputDevice.h>
#include <LWPlatform/LWWindow.h>
#include <LWCore/LWMath.h>
#include "App.h"

State &State_Game::Update(App *A, bool Tick, uint64_t lCurrentTime) {
	if (!Tick) return *this;
	if (!m_Game.Finished()) {
		for (uint32_t i = 0; i < m_BlockerCnt; i++) {
			m_Game.SpawnBlocker(m_Blockers[i].m_Position, m_Blockers[i].m_Theta, m_Blockers[i].m_Blocker);
		}
		m_BlockerCnt = 0;
		if (m_Game.Update(A)) {
			if(!m_Game.Finished()) m_Game.ResetRound();
		}
		m_ScoreLabel->SetTextf("%d | %d", m_Game.GetScoreA(), m_Game.GetScoreB());
	} else {
		if (m_Game.GetScoreA() > m_Game.GetScoreB()) m_ScoreLabel->SetTextf("Red wins!");
		else m_ScoreLabel->SetTextf("Blue wins!");
	}

	return *this;
}

State &State_Game::DrawFrame(Renderer *R, Frame *F, App *A) {
	LWWindow *Wnd = A->GetWindow();
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWVector2f MP = Mouse->GetPositionf();
	float ViewScale = m_Game.GetViewScale(Wnd->GetSizef());
	GameObject Obj = GameObject(MP, 0.0f, Game::BlockSize*ViewScale, m_Theta, m_SelectedBlocker, GameObject::Rect);
	GameObject MiniObj = GameObject(MP, 0.0f, LWVector2f(Game::BlockSize.y, Game::BlockSize.x)*0.25f*A->GetUIManager()->GetLastScale(), 0.0f, m_SelectedBlocker, GameObject::Rect);
	
	m_Game.DrawFrame(F, R, Wnd->GetSizef());
	if (m_Game.Finished() || m_GameMenuRect->GetVisible()) return *this;
	if (m_SelectedBlocker == GameObject::BlockerA && m_Game.GetBlocksA() > 0) {
		Obj.Draw(MP, 1.0f, F, R, LWVector4f(1.0f, 0.0f, 0.0f, 0.6f));
	} else if (m_SelectedBlocker == GameObject::BlockerB && m_Game.GetBlocksB() > 0) {
		Obj.Draw(MP, 1.0f, F, R, LWVector4f(0.0f, 0.0f, 1.0f, 0.6f));
	}

	LWVector2f BlockA = m_ScoreLabel->GetVisiblePosition() + LWVector2f(-MiniObj.GetSize().x*2.0f - 15.0f, m_ScoreLabel->GetVisibleSize().y-MiniObj.GetSize().y);
	LWVector2f BlockB = m_ScoreLabel->GetVisiblePosition() + LWVector2f(m_ScoreLabel->GetVisibleSize().x+MiniObj.GetSize().x*2.0f+15.0f, m_ScoreLabel->GetVisibleSize().y - MiniObj.GetSize().y);
	for (uint32_t i = 0; i < m_Game.GetBlocksA(); i++) {
		MiniObj.Draw(BlockA, 1.0f, F, R, LWVector4f(1.0f, 0.0f, 0.0f, 0.6f));
		BlockA.y -= (BlockA, MiniObj.GetSize().y*2.0f + 5.0f);
	}
	for (uint32_t i = 0; i < m_Game.GetBlocksB(); i++) {
		MiniObj.Draw(BlockB, 1.0f, F, R, LWVector4f(0.0f, 0.0f, 1.0f, 0.6f));
		BlockB.y -= (MiniObj.GetSize().y*2.0f + 5.0f);
	}

	return *this;
}

State &State_Game::ProcessInput(LWWindow *Wnd, App *A) {
	LWMouse *Mouse = Wnd->GetMouseDevice();
	LWEUIManager *UIMan = A->GetUIManager();
	if (Mouse) {
		if (UIMan->GetOverCount() == 0) {
			if (Mouse->ButtonPressed(LWMouseKey::Left)) {
				PushBlocker(m_Game.MapScreen(Mouse->GetPositionf(), Wnd->GetSizef()), m_Theta, m_SelectedBlocker);
			}
			if (Mouse->ButtonPressed(LWMouseKey::Right)) {
				m_SelectedBlocker = (m_SelectedBlocker == GameObject::BlockerA) ? GameObject::BlockerB : GameObject::BlockerA;
			}
		}

		if (Mouse->GetScroll()) {
			if (Mouse->GetScroll() > 0.0f) m_Theta += 0.1f;
			else m_Theta -= 0.1f;
		}
	}
	return *this;
}

State &State_Game::Activate(App *A) {
	LWWindow *Wnd = A->GetWindow();
	m_Game.Initiate(LWVector2f(1280.0f, 720.0f));
	m_GameRect->SetVisible(true);
	m_GameMenuRect->SetVisible(false);
	m_BlockerCnt = 0;
	return *this;
}

State &State_Game::Deactivated(App *A) {
	m_GameRect->SetVisible(false);
	m_GameMenuRect->SetVisible(false);
	return *this;
}

void State_Game::GameMenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	m_GameRect->SetVisible(false);
	m_GameMenuRect->SetVisible(true);
	return;
}

void State_Game::GameMenuBackBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	m_GameRect->SetVisible(true);
	m_GameMenuRect->SetVisible(false);
	return;
}

void State_Game::GameMenuMenuBtnPressed(LWEUI *UI, uint32_t EventCode, void *UserData) {
	App *A = (App*)UserData;
	A->SetActiveState(State::Menu);
	return;
}
bool State_Game::PushBlocker(const LWVector2f &Position, float Theta, uint32_t BlockerID) {
	if (m_BlockerCnt >= MaxBlockerQueue) return false;
	m_Blockers[m_BlockerCnt++] = { Position, BlockerID, Theta };
	return true;
}

State_Game::State_Game(LWEUIManager *UIMan, App *A) : m_Theta(0.0f), m_SelectedBlocker(GameObject::BlockerA) {
	m_GameRect = UIMan->GetNamedUI("GameRect");
	m_GameMenuRect = UIMan->GetNamedUI("GameMenuRect");
	m_ScoreLabel = (LWEUILabel*)UIMan->GetNamedUI("GameScoreLbl");

	UIMan->RegisterMethodEvent("GameMenuBtn", LWEUI::Event_Released, &State_Game::GameMenuBtnPressed, this, A);
	UIMan->RegisterMethodEvent("GameMenu_BackBtn", LWEUI::Event_Released, &State_Game::GameMenuBackBtnPressed, this, A);
	UIMan->RegisterMethodEvent("GameMenu_MenuBtn", LWEUI::Event_Released, &State_Game::GameMenuMenuBtnPressed, this, A);
}