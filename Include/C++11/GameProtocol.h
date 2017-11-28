#ifndef GAMEPROTOCOL_H
#define GAMEPROTOCOL_H
#include <LWCore/LWAllocator.h>
#include <LWNetwork/LWProtocol.h>
#include <unordered_map>
#include <LWCore/LWTypes.h>
#include <functional>

struct GameClient {
	LWSocket *m_Socket;
};

class GameProtocol : public LWProtocol {
public:
	enum{
		PacketBufferSize=1024*256,
		PoolMaxSize=2048,

		PingPacketID = 1,
		PongPacketID

	};

	virtual LWProtocol &Read(LWSocket &Socket, LWProtocolManager *Manager);

	virtual LWProtocol &SocketChanged(LWSocket &Prev, LWSocket &New, LWProtocolManager *Manager);

	virtual LWProtocol &SocketClosed(LWSocket &Socket, LWProtocolManager *Manager);

	bool ReceivedPacket(LWPacket *Pack, LWPacketManager *Manager);

	bool ProcessData(LWSocket &Socket, const char *Buffer, uint32_t BufferLen);

	bool ProcessOutPackets(uint64_t lCurrentTime);

	bool PushOutPacket(LWPacket *Packet, GameClient *Cli);

	template<class Type, class... Args>
	LWPacket *CreatePacket(Args&&... A){
		return m_OutPacketAllocator.Allocate<Type>(std::forward<Args>(A)...);
	}

	bool PopRecvPool(LWPacket **Packet);

	GameClient *GetGameClient(uint32_t IP, uint16_t Port);

	GameProtocol(uint32_t ProtocolID, LWAllocator &Allocator, LWAllocator &InPackets, LWAllocator &OutPackets, std::function<bool(LWPacket *, LWPacketManager*)> PacketSendFunc);

	~GameProtocol();
private:
	LWPacket *m_RecvPool[PoolMaxSize];
	std::unordered_map<uint64_t, GameClient*> m_ClientMap;
	LWAllocator &m_Allocator;
	LWAllocator &m_OutPacketAllocator;
	LWPacketManager *m_PacketManager;
	
	uint32_t m_RecvPoolRead;
	uint32_t m_RecvPoolWrite;

	uint32_t m_SendPoolRead;
	uint32_t m_SendPoolWrite;
	uint32_t m_ProtocolID;

};

#endif