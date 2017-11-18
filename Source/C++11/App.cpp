#include "App.h"
#include <LWCore/LWTimer.h>
#include <LWPlatform/LWVideoMode.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWPlatform/LWFileStream.h>
#include <LWEAsset.h>
#include <LWEUIManager.h>
#include "Renderer.h"
#include "State_Menu.h"
#include "State_Game.h"
#include "State_Game_AI.h"
#include "State_Game_Network.h"

float App::Random(float Min, float Max) {
	float v = (float)(m_RandGen() % 100000) / 100000.0f;
	return Min + (Max - Min)*v;
}

App &App::UpdateThread(uint64_t lCurrentTime) {
	const uint64_t TickFreq = LWTimer::GetResolution() / 60;

	bool Tick = lCurrentTime >= m_NextTick;
	if (Tick) {
		m_NextTick += TickFreq;
		m_FrameReady = true;
	}
	m_States[m_ActiveState]->Update(this, Tick, lCurrentTime);
	if (!m_FrameReady) return *this;
	Frame *F = m_Renderer->BeginFrame();
	if (F) {
		m_UIManager->Draw(&F->m_UIFrame, lCurrentTime);
		m_States[m_ActiveState]->DrawFrame(m_Renderer, F, this);
		m_Renderer->FrameFinished();
		m_FrameReady = false;
	}
	return *this;
}

App &App::InputThread(uint64_t lCurrentTime) {
	LWKeyboard *KB = m_Window->GetKeyboardDevice();

	m_Window->Update(lCurrentTime);
	if (m_Window->GetFlag()&LWWindow::Terminate) m_Flag |= Terminate;
	if (KB->ButtonDown(LWKey::Esc)) m_Flag |= Terminate;
	m_UIManager->Update(lCurrentTime);
	m_States[m_ActiveState]->ProcessInput(m_Window, this);
	return *this;
}

App &App::SetActiveState(uint32_t State) {
	m_States[m_ActiveState]->Deactivated(this);
	m_ActiveState = State;
	m_States[m_ActiveState]->Activate(this);
	return *this;
}

App &App::RenderThread(uint64_t lCurrentTime) {
	m_Renderer->Render(m_Window);
	return *this;
}

bool App::LoadUIData(LWVideoDriver *Driver) {
	char Buffer[64*1024];
	LWEAssetManager *OldAsset = m_AssetManager;
	LWEUIManager *OldManager = m_UIManager;

	LWFileStream F;
	if (!LWFileStream::OpenStream(F, "App:UIData.xml", LWFileStream::ReadMode | LWFileStream::BinaryMode, m_Allocator)) {
		std::cout << "Error reading: 'App:UIData.xml'" << std::endl;
		return false;
	}
	F.ReadText(Buffer, sizeof(Buffer));
	LWEXML *X = m_Allocator.Allocate<LWEXML>();
	if (!LWEXML::ParseBuffer(*X, m_Allocator, Buffer, true)) {
		std::cout << "Error parsing: 'App:UIData.xml'" << std::endl;
		LWAllocator::Destroy(X);
		return false;
	}
	LWEAssetManager *Asset = m_Allocator.Allocate<LWEAssetManager>(Driver, nullptr, m_Allocator);
	LWEUIManager *UIMan = m_Allocator.Allocate<LWEUIManager>(m_Window, &m_Allocator, nullptr, Asset);

	X->PushParser("AssetManager", LWEAssetManager::XMLParser, Asset);
	X->PushParser("UIManager", LWEUIManager::XMLParser, UIMan);
	X->Process();

	m_AssetManager = Asset;
	m_UIManager = UIMan;

	LWAllocator::Destroy(OldAsset);
	LWAllocator::Destroy(OldManager);
	LWAllocator::Destroy(X);
	return true;
}

uint32_t App::GetFlag(void) const {
	return m_Flag;
}

LWEUIManager *App::GetUIManager(void) {
	return m_UIManager;
}

LWWindow *App::GetWindow(void) {
	return m_Window;
}

App::App(LWAllocator &Allocator) : m_Allocator(Allocator), m_Window(nullptr), m_Renderer(nullptr), m_AssetManager(nullptr), m_UIManager(nullptr), m_Flag(0), m_NextTick(LWTimer::GetCurrent()), m_FrameReady(false) {
	std::seed_seq sd = { LWTimer::GetCurrent() };
	m_RandGen.seed(sd);
	LWVideoMode Current = LWVideoMode::GetActiveMode();
	LWVector2i DeskSize = Current.GetSize();
	LWVector2i WndSize = LWVector2i(1280, 720);
	if (!LWProtocolManager::InitateNetwork()) {
		m_Flag |= Terminate;
		return;
	}
	
	char DriverNames[][32] = { "OpenGL 3.2", "OpenGL 2.1", "DirectX 11.1", "OpenGLES 2", "DirectX 12", "OpenGL 4.5", "OpenGLES3", "Metal", "Vulkan" };
	char PlatformNames[][32] = LWPLATFORM_NAMES;
	char ArchNames[][32] = LWARCH_NAMES;
	memset(m_States, 0, sizeof(m_States));
	m_Window = m_Allocator.Allocate<LWWindow>("Pong Breaker", "PongBreaker", m_Allocator, LWWindow::WindowedMode | LWWindow::MouseDevice | LWWindow::KeyboardDevice, DeskSize / 2 - WndSize / 2, WndSize);

	if (!m_Window) {
		m_Flag |= Terminate;
		return;
	}

	LWVideoDriver *Driver = LWVideoDriver::MakeVideoDriver(m_Window);
	if (!Driver) {
		std::cout << "Issue making driver." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	uint32_t DriverType = (uint32_t)(log(Driver->GetDriverType()) / log(2));
	m_Window->SetTitlef("Pong Breaker | %s | %s | %s", DriverNames[DriverType], PlatformNames[LWPLATFORM_ID], ArchNames[LWARCH_ID]);


	if (!LoadUIData(Driver)) {
		std::cout << "Error loading data." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	m_Renderer = m_Allocator.Allocate<Renderer>(Driver, m_AssetManager, m_Allocator);

	m_States[State::Menu] = m_Allocator.Allocate<State_Menu>();
	m_States[State::GameS] = m_Allocator.Allocate<State_Game>(m_UIManager, this);
	m_States[State::GameAI] = m_Allocator.Allocate<State_Game_AI>(m_UIManager, this);
	m_States[State::GameNetwork] = m_Allocator.Allocate<State_Game_Network>(m_UIManager, this);
	m_ActiveState = State::GameAI;
	m_States[m_ActiveState]->Activate(this);
}

App::~App() {
	LWAllocator::Destroy((State_Menu*)m_States[State::Menu]);
	LWAllocator::Destroy((State_Game*)m_States[State::GameS]);
	LWAllocator::Destroy((State_Game_AI*)m_States[State::GameAI]);
	LWAllocator::Destroy((State_Game_Network*)m_States[State::GameNetwork]);
	LWAllocator::Destroy(m_UIManager);
	LWAllocator::Destroy(m_AssetManager);
	LWAllocator::Destroy(m_Renderer);
	LWAllocator::Destroy(m_Window);
	LWProtocolManager::TerminateNetwork();
}