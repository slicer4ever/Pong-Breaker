#ifndef PROTOCOL_WEBSOCKET_H
#define PROTOCOL_WEBSOCKET_H
#include <LWNetwork/LWProtocol.h>
#include <LWNetwork/LWProtocolManager.h>
#define WEBSOCKET_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

struct LWEWebSocket {
	enum{
		CONNECTING,
		CONNECTED
	};
	uint32_t m_Status;
	char m_Path[128];
	char m_Origin[128];
	char m_SecKey[128];
	char m_SecProtocols[128];
	LWSocket *m_Socket;
};

class LWEProtocolWebSocket : public LWProtocol {
public:
	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	bool ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen);

	LWEProtocolWebSocket &SetServer(const char *Server);

	LWEProtocolWebSocket &SetUserAgent(const char *Server);

	LWEProtocolWebSocket(uint32_t ProtocolID, LWProtocolManager *Manager);

protected:
	char m_Server[256];
	char m_UserAgent[256];
	uint32_t m_ProtocolID;
	LWProtocolManager *m_Manager;
};

#endif