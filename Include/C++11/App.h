#ifndef APP_H
#define APP_H
#include <LWCore/LWAllocator.h>
#include <LWPlatform/LWWindow.h>
#include <LWVideo/LWVideoDriver.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWETypes.h>
#include "GameProtocol.h"
#include "Types.h"
#include "State.h"
#include <random>

class App {
public:
	enum {
		WebSocketPort = 1033,
		GameSocketPort = 1034,

		GameProtocolID = 0,

		Terminate = 0x1
	};

	float Random(float Min, float Max);

	App &NetworkThread(uint64_t lCurrentTime);

	App &UpdateThread(uint64_t lCurrentTime);

	App &InputThread(uint64_t lCurrentTime);

	App &RenderThread(uint64_t lCurrentTime);

	App &SetActiveState(uint32_t State);

	bool SendPacket(LWPacket *Packet, LWPacketManager *PacketManager);

	bool LoadUIData(LWVideoDriver *Driver);

	uint32_t GetFlag(void) const;

	LWEUIManager *GetUIManager(void);

	LWWindow *GetWindow(void);

	App(LWAllocator &Allocator, LWAllocator &InPacketAlloc, LWAllocator &OutPacketAlloc);

	~App();
private:
	LWProtocolManager *m_ProtocolManager;
	std::mt19937 m_RandGen;
	LWAllocator &m_Allocator;
	State *m_States[State::Count];
	GameClient *m_ServerClient;
	LWWindow *m_Window;
	LWEUIManager *m_UIManager;
	LWEAssetManager *m_AssetManager;
	GameProtocol *m_GameProtocol;
	Renderer *m_Renderer;
	uint64_t m_NextTick;
	uint32_t m_Flag;
	uint32_t m_ActiveState;
	bool m_FrameReady;
};

#endif
