#include "ServerApp.h"
#include <LWCore/LWText.h>
#include <iostream>

ServerApp &ServerApp::NetworkSend(uint64_t lCurrentTime) {
	m_HttpProtocol.ProcessRequests(HttpProtocolID, m_ProtocolManager);
	m_HttpsProtocol->ProcessRequests(HttpsProtocolID, m_ProtocolManager);
	m_WebProtocol->ProcessOutPackets();
	m_WebSecureProtocol->ProcessOutPackets();
	/*
	if(!m_Sent){
		LWText T("GET /\n\n");
		uint32_t res = m_TLSProtocol->Send(*m_TLSSock, (const char*)T.GetCharacters(), T.GetLength()+1);
		if(res==-1){
			std::cout << "Connection was closed." << std::endl;
			m_Sent = true;
		}else if(res){
			std::cout << "Sent data: " << res << std::endl;
			m_Sent = true;
		}
	}*/
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
	char IPBuf[32];
	LWEHttpRequest Request;
	LWEWebPacket Packet;
	while(m_HttpProtocol.GetNextRequest(Request)){
		Request.SetContentType("text/html");
		Request.m_Flag |= LWEHttpRequest::ConnectionClose;
		m_HttpProtocol.PushResponse(Request, "<html><body>Hello World!</body></html>", LWEHttpRequest::Ok);
	}
	while(m_HttpsProtocol->GetNextRequest(Request)){
		Request.SetContentType("text/html");
		Request.m_Flag |= LWEHttpRequest::ConnectionClose;
		m_HttpsProtocol->PushResponse(Request, "<html><body>Hello World!</body></html>", LWEHttpRequest::Ok);
	}
	while (m_WebProtocol->GetNextPacket(Packet)) {
		LWEWebSocket *WebSock = Packet.m_WebSocket;
		LWSocket::MakeAddress(WebSock->m_Socket->GetRemoteIP(), IPBuf, sizeof(IPBuf));
		std::cout << "Got packet from: " << IPBuf << ":" << WebSock->m_Socket->GetRemotePort() << " Data: " << Packet.m_DataLen << " Type: " << Packet.GetOp() << " Finished: " << Packet.Finished() << std::endl;
		if (Packet.GetOp() == LWEWebPacket::CONTROL_TEXT) {
			std::cout << "Data: '" << Packet.m_Data << "'" << std::endl;
		} else std::cout << "Binary data." << std::endl;
		m_WebProtocol->PushOutPacket(Packet.m_Data, Packet.m_DataLen, WebSock, Packet.GetOp());
		Packet.WorkFinished();
	}
	while (m_WebSecureProtocol->GetNextPacket(Packet)) {
		LWEWebSocket *WebSock = Packet.m_WebSocket;
		LWSocket::MakeAddress(WebSock->m_Socket->GetRemoteIP(), IPBuf, sizeof(IPBuf));
		std::cout << "Got packet from: " << IPBuf << ":" << WebSock->m_Socket->GetRemotePort() << " Data: " << Packet.m_DataLen << " Type: " << Packet.GetOp() << " Finished: " << Packet.Finished() << std::endl;
		if (Packet.GetOp() == LWEWebPacket::CONTROL_TEXT) {
			std::cout << "Data: '" << Packet.m_Data << "'" << std::endl;
		} else std::cout << "Binary data." << std::endl;
		m_WebSecureProtocol->PushOutPacket(Packet.m_Data, Packet.m_DataLen, WebSock, Packet.GetOp());
		Packet.WorkFinished();
	}
	return *this;
}

uint32_t ServerApp::GetFlag(void) const {
	return m_Flag;
}

bool ServerApp::WebSockClosed(LWSocket &Socket, LWEWebSocket *WebSock, LWProtocolManager *Manager) {
	std::cout << "My web sockets been closed!" << std::endl;
	return true;
}

ServerApp::ServerApp(LWAllocator &Allocator, LWAllocator &PacketSendAlloc, LWAllocator &PacketRecvAlloc) : m_Allocator(Allocator), m_Flag(0) {
	char IPBuf[32];
	if (!LWProtocolManager::InitateNetwork()) {
		std::cout << "Error starting network." << std::endl;
		m_Flag |= Terminate;
		return;
	}
	m_HttpProtocol.SetProtocolID(HttpProtocolID).SetProtocolManager(&m_ProtocolManager);
	m_HttpsProtocol = m_Allocator.Allocate<LWEProtocolHttps>(HttpsProtocolID, TLSProtocolID, &m_ProtocolManager, m_Allocator, "App:realkey/fullchain.pem", "App:realkey/privkey.pem");
	m_HttpsProtocol->SetServerString("Neonlightgames.com");

	m_WebProtocol = m_Allocator.Allocate<LWEProtocolWebSocket>(WebProtocolID, m_Allocator, &m_ProtocolManager);
	m_WebProtocol->SetWebSocketClosedCallbackMethod(&ServerApp::WebSockClosed, this);

	m_WebSecureProtocol = m_Allocator.Allocate<LWEProtocolWebSocketSecure>(0, 1, m_Allocator, &m_ProtocolManager, "App:realkey/fullchain.pem", "App:realkey/privkey.pem");
	m_WebSecureProtocol->SetWebSocketClosedCallbackMethod(&ServerApp::WebSockClosed, this);

	m_ProtocolManager.RegisterProtocol(m_WebProtocol, WebProtocolID);
	m_ProtocolManager.RegisterProtocol(&m_HttpProtocol, HttpProtocolID);
	m_ProtocolManager.RegisterProtocol(m_HttpsProtocol, HttpsProtocolID);
	m_ProtocolManager.RegisterProtocol(m_WebSecureProtocol, WebSProtocolID);

	LWSocket Sock;
	uint32_t Err = LWSocket::CreateSocket(Sock, 3000, LWSocket::Tcp | LWSocket::Listen, HttpProtocolID);
	if(Err){
		std::cout << "Error creating httpsocket listener: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}
	LWSocket *HttpListener = m_ProtocolManager.PushSocket(Sock);
	
	Err = LWSocket::CreateSocket(Sock, (uint16_t)3001, LWSocket::Tcp | LWSocket::Listen, HttpsProtocolID);
	if (Err) {
		std::cout << "Error creating Https socket: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}
	LWSocket *HttpsListener = m_ProtocolManager.PushSocket(Sock);

	Err = LWSocket::CreateSocket(Sock, (uint16_t)3002, LWSocket::Tcp | LWSocket::Listen, WebProtocolID);
	if (Err) {
		std::cout << "Error creating web socket: " << Err << std::endl;
		m_Flag |= Terminate;
		return;
	}
	LWSocket *WebListener = m_ProtocolManager.PushSocket(Sock);

	Err = LWSocket::CreateSocket(Sock, (uint16_t)3003, LWSocket::Tcp | LWSocket::Listen, WebSProtocolID);
	if (Err) {
		std::cout << "Error creating web socket: " << ERROR << std::endl;
		m_Flag |= Terminate;
		return;
	}

	LWSocket *WebSListener = m_ProtocolManager.PushSocket(Sock);

	std::cout << "Httpsocket listening at: " << HttpListener->GetLocalPort() << std::endl;
	std::cout << "Httpssocket listener at: " << HttpsListener->GetLocalPort() << std::endl;
	std::cout << "Websocket listening at: " << WebListener->GetLocalPort() << std::endl;
	std::cout << "WebSocketSecure listening at: " << WebSListener->GetLocalPort() << std::endl;
}

ServerApp::~ServerApp() {
	LWProtocolManager::TerminateNetwork();
}
