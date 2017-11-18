#include "Protocol_WebSocket.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <iostream>

LWProtocol &Protocol_WebSocket::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char IPBuf[32];
	char Buffer[64 * 1024]; //64 kb buffer!
	if (&Socket == m_Listener) {
		LWSocket Accepted;
		if (!m_Listener->Accept(Accepted)) {
			std::cout << "Error accepting socket!" << std::endl;
			return *this;
		}
		LWSocket::MakeAddress(Accepted.GetRemoteIP(), IPBuf, sizeof(IPBuf));
		std::cout << "New connection from: " << IPBuf << ":" << Accepted.GetRemotePort() << std::endl;
		Manager->PushSocket(Accepted);
	} else {
		uint32_t r = Socket.Receive(Buffer, sizeof(Buffer));
		if (r == 0xFFFFFFFF) {
			Socket.MarkClosable();
			return *this;
		}
		std::cout << "Got data len: " << r << std::endl;
	}
	return *this;
}

LWProtocol &Protocol_WebSocket::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	char IPBuf[32];
	if (&Socket == m_Listener) {
		std::cout << "Listener socket closed." << std::endl;
		m_Listener = nullptr;
	} else {
		LWSocket::MakeAddress(Socket.GetRemoteIP(), IPBuf, sizeof(IPBuf));
		std::cout << "Connection closed: " << IPBuf << ":" << Socket.GetRemotePort() << std::endl;
	}

	return *this;
}

LWProtocol &Protocol_WebSocket::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	if (&Prev == m_Listener) m_Listener = &New;
	return *this;
}

Protocol_WebSocket &Protocol_WebSocket::SetListener(LWSocket *Sock) {
	m_Listener = Sock;
	return *this;
}

Protocol_WebSocket::Protocol_WebSocket() : m_Listener(nullptr) {}