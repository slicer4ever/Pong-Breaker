#include "ServerApp.h"
#include "GameProtocol.h"
#include <LWCore/LWText.h>
#include <LWNetwork/LWPacketManager.h>
#include <LWNetwork/LWPacket.h>
#include <iostream>

ServerApp &ServerApp::NetworkSend(uint64_t lCurrentTime) {
	m_WebProtocol->ProcessOutPackets();
	m_GameProtocol->ProcessOutPackets(lCurrentTime);
	return *this;
}

ServerApp &ServerApp::NetworkRecv(uint64_t lCurrentTime) {
	if (!m_ProtocolManager->Poll(0)) {
		std::cout << "Error polling!" << std::endl;
		m_Flag |= Terminate;
	}
	return *this;
}

ServerApp &ServerApp::Update(uint64_t lCurrentTime) {
	char IPBuf[32];
	LWEWebPacket Packet;
	LWPacket *Pack;
	while (m_WebProtocol->GetNextPacket(Packet)) {
		LWEWebSocket *WebSock = Packet.m_WebSocket;
		if (Packet.GetOp() != LWEWebPacket::CONTROL_BINARY) {
			std::cout << "Received non-binary packet." << std::endl;
			continue;
		}
		m_GameProtocol->ProcessData(*WebSock->m_Socket, Packet.m_Data, Packet.m_DataLen);
		Packet.WorkFinished();
	}
	while(m_GameProtocol->PopRecvPool(&Pack)){
		//Parse packets!
		for (LWPacket *P = Pack; P; P = P->GetNext()) {
			std::cout << "Got Packet: " << P->GetMajorType() << " | " << P->GetType() << std::endl;
		}
	}
	return *this;
}

uint32_t ServerApp::GetFlag(void) const {
	return m_Flag;
}

bool ServerApp::SendPacket(LWPacket *Pack, LWPacketManager *Manager) {
	char Buffer[1024 * 256];
	uint32_t BufferLen = Manager->SerializePacket(Pack, Buffer, sizeof(Buffer));
	GameClient *Cli = (GameClient*)Pack->GetClient();
	LWSocket *Sock = Cli->m_Socket;
	if (!Sock) return true;
	LWEWebSocket *WebSock = (LWEWebSocket*)Sock->GetProtocolData(WebProtocolID);
	if (!WebSock) {
		uint32_t o = 0;
		while (o != BufferLen) {
			uint32_t res = Sock->Send(Buffer + o, BufferLen - o);
			if(res==-1){
				std::cout << "Error sending data." << std::endl;
				return true;
			}
			o += res;
		}
	}else m_WebProtocol->PushOutPacket(Buffer, BufferLen, WebSock, LWEWebPacket::CONTROL_BINARY);

	return true;
}

bool ServerApp::WebSockClosed(LWSocket &Socket, LWEWebSocket *WebSock, LWProtocolManager *Manager) {
	m_GameProtocol->SocketClosed(Socket, Manager);
	return true;
}
#include <LWCore/LWCrypto.h>
ServerApp::ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc) : m_Allocator(Allocator), m_Flag(0) {
	if (!LWProtocolManager::InitateNetwork()) {
		std::cout << "Error starting network." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	
	m_ProtocolManager = m_Allocator.Allocate<LWProtocolManager>();
	m_WebProtocol = m_Allocator.Allocate<LWEProtocolWebSocket>(0, m_Allocator, m_ProtocolManager);
	m_WebProtocol->SetWebSocketClosedCallbackMethod(&ServerApp::WebSockClosed, this);
	m_WebProtocol->SetSubProtocol("binary").SetServer("neonlightgames.com");
	m_GameProtocol = m_Allocator.Allocate<GameProtocol>(GameProtocolID, m_Allocator, PacketRecvAlloc, PacketSendAlloc, std::bind(&ServerApp::SendPacket, this, std::placeholders::_1, std::placeholders::_2));

	m_ProtocolManager->RegisterProtocol(m_WebProtocol, WebProtocolID);
	m_ProtocolManager->RegisterProtocol(m_GameProtocol, GameProtocolID);

	LWSocket Sock;	
	uint32_t Err = LWSocket::CreateSocket(Sock, (uint16_t)WebSocketPort, LWSocket::Tcp | LWSocket::Listen, WebProtocolID);
	if (Err) {
		std::cout << "Error creating web socket: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}
	LWSocket *WebSListener = m_ProtocolManager->PushSocket(Sock);

	Err = LWSocket::CreateSocket(Sock, (uint16_t)SocketPort, LWSocket::Tcp | LWSocket::Listen, GameProtocolID);
	if(Err){
		std::cout << "Error creating game socket: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}

	LWSocket *GameListener = m_ProtocolManager->PushSocket(Sock);

	std::cout << "WebSocketSecure listening at: " << WebSListener->GetLocalPort() << std::endl;
	std::cout << "GameProtocol listening at: " << GameListener->GetLocalPort() << std::endl;

	char Hashed[256];
	char Buf[20];

	char *Str = "RCD4KnTzJ3osYyu9hsORVw==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	LWCrypto::HashSHA1(Str, strlen(Str), Buf);
	uint32_t *t = (uint32_t*)Buf;
	for (uint32_t i = 0; i < 5; i++) t[i] = (t[i] & 0xFF) << 24 | (t[i] & 0xFF00) << 8 | (t[i] & 0xFF0000) >> 8 | (t[i] & 0xFF000000) >> 24;
	uint32_t Len = LWCrypto::Base64Encode(Buf, 20, Hashed, 256);
	Hashed[Len] = '\0';
	std::cout << "'" << Hashed << "'" << std::endl;

}

ServerApp::~ServerApp() {
	LWAllocator::Destroy(m_ProtocolManager);
	LWAllocator::Destroy(m_WebProtocol);
	LWAllocator::Destroy(m_GameProtocol);
	LWProtocolManager::TerminateNetwork();
}
