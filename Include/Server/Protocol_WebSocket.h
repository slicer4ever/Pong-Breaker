#ifndef PROTOCOL_WEBSOCKET_H
#define PROTOCOL_WEBSOCKET_H
#include <LWNetwork/LWProtocol.h>

class Protocol_WebSocket : public LWProtocol {
public:
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	Protocol_WebSocket &SetListener(LWSocket *Sock);

	Protocol_WebSocket();

protected:
	LWSocket *m_Listener;
};

#endif