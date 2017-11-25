#include "LWEProtocolWebSocket.h"
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <iostream>

LWProtocol &LWEProtocolWebSocket::Read(LWSocket &Socket, LWProtocolManager *Manager) {
	char IPBuf[32];
	char Buffer[64 * 1024]; //64 kb buffer!
	if (Socket.GetFlag()&LWSocket::Listen){
		LWSocket Accepted;
		if (!Socket.Accept(Accepted)) {
			std::cout << "Error accepting socket!" << std::endl;
			return *this;
		}
		LWSocket::MakeAddress(Accepted.GetRemoteIP(), IPBuf, sizeof(IPBuf));
		std::cout << "New connection from: " << IPBuf << ":" << Accepted.GetRemotePort() << std::endl;
		Manager->PushSocket(Accepted);
		return *this;
	}
	uint32_t r = Socket.Receive(Buffer, sizeof(Buffer));
	if (r == 0xFFFFFFFF) {
		Socket.MarkClosable();
		return *this;
	}
	ProcessRead(Socket, Buffer, r);
	return *this;
}

bool LWEProtocolWebSocket::ProcessRead(LWSocket &Socket, const char *Buffer, uint32_t BufferLen) {
	
}

LWProtocol &LWEProtocolWebSocket::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager) {
	LWAllocator::Destroy((LWEWebSocket*)Socket.GetProtocolData(m_ProtocolID));

	return *this;
}

LWProtocol &LWEProtocolWebSocket::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager) {
	New.SetProtocolData(m_ProtocolID, Prev.GetProtocolData(m_ProtocolID));
	return *this;
}

LWEProtocolWebSocket::LWEProtocolWebSocket(uint32_t ProtocolID, LWProtocolManager *Manager) : m_ProtocolID(ProtocolID), m_Manager(Manager){

}