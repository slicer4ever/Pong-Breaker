#include "GameProtocol.h"
#include <LWCore/LWTimer.h>
#include <LWNetwork/LWSocket.h>
#include <LWNetwork/LWProtocolManager.h>
#include <LWNetwork/LWPacketManager.h>
#include <LWNetwork/LWPacket.h>
#include <functional>
#include <iostream>

LWProtocol &GameProtocol::Read(LWSocket &Socket, LWProtocolManager *Manager){
	char Buffer[256 * 1024];
	if(Socket.GetFlag()&LWSocket::Listen){
		LWSocket Sock;
		if(!Socket.Accept(Sock)){
			std::cout << "Error accepting socket." << std::endl;
		}
		LWSocket *rSock = Manager->PushSocket(Sock);
		GameClient *Cli = GetGameClient(rSock->GetRemoteIP(), rSock->GetRemotePort());
		Cli->m_Socket = rSock;
		rSock->SetProtocolData(m_ProtocolID, Cli);
		return *this;
	}
	uint32_t Res = Socket.Receive(Buffer, sizeof(Buffer));
	if (Res == -1) {
		std::cout << "Error receiving data." << std::endl;
		Socket.MarkClosable();
		return *this;
	}else if(Res==0){
		Socket.MarkClosable();
		return *this;
	}
	ProcessData(Socket, Buffer, Res);
	return *this;
}

bool GameProtocol::ProcessData(LWSocket &Socket, const char *Buffer, uint32_t BufferLen) {
	GameClient *Cli = (GameClient*)Socket.GetProtocolData(m_ProtocolID);
	if(!Cli){
		Cli = GetGameClient(Socket.GetRemoteIP(), Socket.GetRemotePort());
		Cli->m_Socket = &Socket;
		Socket.SetProtocolData(m_ProtocolID, Cli);
	}
	if(!m_PacketManager->ProcessRawData(Cli, &Socket, Buffer, BufferLen)){
		std::cout << "Error processing raw data." << std::endl;
		return false;
	}
	return true;
}

bool GameProtocol::ProcessOutPackets(uint64_t lCurrentTime){
	m_PacketManager->Update(lCurrentTime, LWTimer::GetResolution());
	return true;
}

bool GameProtocol::PushOutPacket(LWPacket *Packet, GameClient *Cli){
	if (m_SendPoolWrite - m_SendPoolRead >= PoolMaxSize) return false;
	Packet->SetClient(Cli);
	m_PacketManager->PushOutPacket(Packet);
	return true;
}

bool GameProtocol::ReceivedPacket(LWPacket *Pack, LWPacketManager *Manager) {
	if (m_RecvPoolWrite - m_RecvPoolRead >= PoolMaxSize) return false;
	m_RecvPool[m_RecvPoolWrite%PoolMaxSize] = Pack;
	m_RecvPoolWrite++;
	return true;
}

bool GameProtocol::PopRecvPool(LWPacket **Packet){
	if (m_RecvPoolWrite == m_RecvPoolRead) return false;
	*Packet = m_RecvPool[m_RecvPoolRead%PoolMaxSize];
	m_RecvPoolRead++;
	return true;
}

LWProtocol &GameProtocol::SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager){
	GameClient *Cli = (GameClient*)Prev.GetProtocolData(m_ProtocolID);
	New.SetProtocolData(m_ProtocolID, Cli);
	if(Cli) Cli->m_Socket = &New;
	return *this;
}

LWProtocol &GameProtocol::SocketClosed(LWSocket &Socket, LWProtocolManager *Manager){
	GameClient *Cli = (GameClient*)Socket.GetProtocolData(m_ProtocolID);
	if(Cli) Cli->m_Socket = nullptr;
	return *this;
}

GameClient *GameProtocol::GetGameClient(uint32_t IP, uint16_t Port){
	uint64_t Hash = (uint64_t)IP | (((uint64_t)Port) << 32);
	auto Iter = m_ClientMap.find(Hash);
	if(Iter==m_ClientMap.end()){
		GameClient *Cli = m_Allocator.Allocate<GameClient>();
		std::pair<uint64_t, GameClient*> Pair(Hash, Cli);
		m_ClientMap.insert(Pair);
		Cli->m_Socket = nullptr;
		return Cli;
	}
	return Iter->second;
}

GameProtocol::GameProtocol(uint32_t ProtocolID, LWAllocator &Allocator, LWAllocator &InPackets, LWAllocator &OutPackets, std::function<bool(LWPacket*, LWPacketManager*)> SendPackketFunc) : LWProtocol(), m_Allocator(Allocator), m_OutPacketAllocator(OutPackets), m_ProtocolID(ProtocolID){
	m_PacketManager = Allocator.Allocate<LWPacketManager>(PacketBufferSize, Allocator, InPackets, std::bind(&GameProtocol::ReceivedPacket, this, std::placeholders::_1, std::placeholders::_2), SendPackketFunc);

}

GameProtocol::~GameProtocol(){
	for(auto &&Iter : m_ClientMap){
		GameClient *Cli = Iter.second;
		LWAllocator::Destroy(Cli);
	}
	LWAllocator::Destroy(m_PacketManager);
}