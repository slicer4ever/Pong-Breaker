#ifndef SERVERAPP_H
#define SERVERAPP_H
#include <LWCore/LWTypes.h>
#include <LWNetwork/LWProtocolManager.h>
#include "Protocol_WebSocket.h"

class ServerApp {
public:
	enum {
		WebSocketPort=1033,
		SocketPort=1034,

		WebSocketID=0,

		Terminate=0x1
	};
	
	ServerApp &NetworkSend(uint64_t lCurrentTime);

	ServerApp &NetworkRecv(uint64_t lCurrentTime);

	ServerApp &Update(uint64_t lCurrentTime);

	uint32_t GetFlag(void) const;
	
	ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc);

	~ServerApp();
private:
	LWProtocolManager m_ProtocolManager;
	Protocol_WebSocket m_WebSocketProtocol;
	LWAllocator &m_Allocator;
	uint32_t m_Flag;
};

#endif