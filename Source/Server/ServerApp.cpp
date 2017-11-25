#include "ServerApp.h"
#include <LWCore/LWText.h>
#include <iostream>

ServerApp &ServerApp::NetworkSend(uint64_t lCurrentTime) {
	m_HttpProtocol.ProcessRequests(HttpProtocolID, m_ProtocolManager);
	m_HttpsProtocol->ProcessRequests(HttpsProtocolID, m_ProtocolManager);
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
	LWEHttpRequest Request;
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
	m_HttpProtocol.SetProtocolID(HttpProtocolID).SetProtocolManager(&m_ProtocolManager);
	m_HttpsProtocol = m_Allocator.Allocate<LWEProtocolHttps>(HttpsProtocolID, TLSProtocolID, &m_ProtocolManager, m_Allocator, "App:livkey/fullchain.pem", "App:livkey/privkey.pem");
	m_HttpsProtocol->SetServerString("Neonlightgames.com");
	m_ProtocolManager.RegisterProtocol(&m_HttpProtocol, HttpProtocolID);
	m_ProtocolManager.RegisterProtocol(m_HttpsProtocol, HttpsProtocolID);

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
	std::cout << "Httpsocket listening at: " << HttpListener->GetLocalPort() << std::endl;
	std::cout << "Httpssocket listener at: " << HttpsListener->GetLocalPort() << std::endl;
}

ServerApp::~ServerApp() {
	LWProtocolManager::TerminateNetwork();
}
