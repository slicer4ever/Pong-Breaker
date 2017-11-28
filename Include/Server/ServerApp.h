#ifndef SERVERAPP_H
#define SERVERAPP_H
#include <LWCore/LWTypes.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWEProtocols/LWEProtocolWebSocket.h>
#include <LWEProtocols/LWEProtocolWebSocketSecure.h>
#include "GameProtocol.h"

class ServerApp {
public:
	enum {
		WebSocketPort=1033,
		SocketPort=1034,

		WebProtocolID=0,
		WebTLSProtocolID,
		GameProtocolID,

		Terminate=0x1
	};
	
	ServerApp &NetworkSend(uint64_t lCurrentTime);

	ServerApp &NetworkRecv(uint64_t lCurrentTime);

	ServerApp &Update(uint64_t lCurrentTime);

	bool SendPacket(LWPacket *Pack, LWPacketManager *Manager);

	bool WebSockClosed(LWSocket &Socket, LWEWebSocket *WebSock, LWProtocolManager *Manager);

	uint32_t GetFlag(void) const;
	
	ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc);

	~ServerApp();
private:
	LWProtocolManager *m_ProtocolManager;
	LWEProtocolWebSocketSecure *m_WebProtocol;
	GameProtocol *m_GameProtocol;
	LWAllocator &m_Allocator;
	uint32_t m_Flag;
	bool m_Sent = false;
};

#endif