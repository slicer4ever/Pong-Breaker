#include "ServerApp.h"
#include <iostream>

ServerApp &ServerApp::NetworkSend(uint64_t lCurrentTime) {
	return *this;
}

ServerApp &ServerApp::NetworkRecv(uint64_t lCurrentTime) {
	if (!m_ProtocolManager.Poll(0)) {
		std::cout << "Error polling!" << std::endl;
		m_Flag |= Terminate;
	}
	return *this;
}

ServerApp &ServerApp::Update(uint64_t lCurrentTime) {
	return *this;
}

uint32_t ServerApp::GetFlag(void) const {
	return m_Flag;
}

ServerApp::ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc) : m_Allocator(Allocator), m_Flag(0) {
	char IPBuf[32];
	if (!LWProtocolManager::InitateNetwork()) {
		std::cout << "Error starting network." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	m_ProtocolManager.RegisterProtocol(&m_WebSocketProtocol, WebSocketID);

	LWSocket WebSocketSock;
	uint32_t Err = LWSocket::CreateSocket(WebSocketSock, WebSocketPort, LWSocket::Tcp | LWSocket::Listen, WebSocketID);
	if(Err){
		std::cout << "Error creating websocket listener: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}
	LWSocket *WebListener = m_ProtocolManager.PushSocket(WebSocketSock);
	m_WebSocketProtocol.SetListener(WebListener);
	std::cout << "Websocket listening at: " << WebListener->GetLocalPort() << std::endl;

}

ServerApp::~ServerApp() {
	LWProtocolManager::TerminateNetwork();
}
