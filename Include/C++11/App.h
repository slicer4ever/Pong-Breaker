#ifndef APP_H
#define APP_H
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWWindow.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWETypes.h>
#include "Types.h"
#include "State.h"
#include <random>

class App {
public:
	enum {
		Terminate = 0x1
	};

	float Random(float Min, float Max);

	App &UpdateThread(uint64_t lCurrentTime);

	App &InputThread(uint64_t lCurrentTime);

	App &RenderThread(uint64_t lCurrentTime);

	App &SetActiveState(uint32_t State);

	bool LoadUIData(LWVideoDriver *Driver);

	uint32_t GetFlag(void) const;

	LWEUIManager *GetUIManager(void);

	LWWindow *GetWindow(void);

	App(LWAllocator &Allocator);

	~App();
private:
	LWProtocolManager m_ProtocolManager;
	std::mt19937 m_RandGen;
	LWAllocator &m_Allocator;
	State *m_States[State::Count];
	LWWindow *m_Window;
	LWEUIManager *m_UIManager;
	LWEAssetManager *m_AssetManager;
	Renderer *m_Renderer;
	uint64_t m_NextTick;
	uint32_t m_Flag;
	uint32_t m_ActiveState;
	bool m_FrameReady;
};

#endif
