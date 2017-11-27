#ifndef SERVERAPP_H
#define SERVERAPP_H
#include <LWCore/LWTypes.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWEProtocols/LWEProtocolHTTP.h>
#include <LWEProtocols/LWEProtocolHTTPS.h>
#include <LWEProtocols/LWEProtocolTLS.h>
#include <LWEProtocols/LWEProtocolWebSocket.h>
#include <LWEProtocols/LWEProtocolWebSocketSecure.h>

class ServerApp {
public:
	enum {
		WebSocketPort=1033,
		SocketPort=1034,

		WebProtocolID=0,
		HttpProtocolID,
		HttpsProtocolID,
		TLSProtocolID,
		WebSProtocolID,


		Terminate=0x1
	};
	
	ServerApp &NetworkSend(uint64_t lCurrentTime);

	ServerApp &NetworkRecv(uint64_t lCurrentTime);

	ServerApp &Update(uint64_t lCurrentTime);

	bool WebSockClosed(LWSocket &Socket, LWEWebSocket *WebSock, LWProtocolManager *Manager);

	uint32_t GetFlag(void) const;
	
	ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc);

	~ServerApp();
private:
	LWProtocolManager m_ProtocolManager;
	LWEProtocolHttp m_HttpProtocol;
	LWEProtocolHttps *m_HttpsProtocol;
	LWEProtocolWebSocket *m_WebProtocol;
	LWEProtocolWebSocketSecure *m_WebSecureProtocol;

	LWEWebSocket *WebSocket;
	LWAllocator &m_Allocator;
	uint32_t m_Flag;
	bool m_Sent = false;
};

#endif