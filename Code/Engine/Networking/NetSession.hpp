/************************************************************************/
/* File: NetSession.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single collection of network connections
/************************************************************************/
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/DataStructures/ThreadSafeVector.hpp"
#include <vector>
#include <string>
#include <mutex>

class UDPSocket;
class NetPacket;
class NetMessage;
class BytePacker;
class NetConnection;
class NetSession;

#define INVALID_CONNECTION_INDEX (0xff)

struct NetSender_t
{
	NetAddress_t	address;
	uint8_t			connectionIndex = INVALID_CONNECTION_INDEX;
	NetSession*		netSession = nullptr;
};


// Enum for message definitions
enum eNetCoreMessage : uint8_t
{
	NET_MSG_PING = 0,
	NET_MSG_PONG,
	NET_MSG_HEARTBEAT,
	NET_MSG_CORE_COUNT
};

enum eNetMessageOption : uint32_t
{
	NET_MSG_OPTION_NONE = 0,
	NET_MSG_OPTION_CONNECTIONLESS = (1 << 0),
	NET_MSG_OPTION_RELIABLE = (1 << 1),
	NET_MSG_OPTION_IN_ORDER = (1 << 2),
	NET_MSG_OPTION_HEARTBEAT = (1 << 3)
};

struct PendingReceive
{
	float			timeStamp = 0;
	NetPacket*		packet = nullptr;
	NetAddress_t	senderAddress;
};


class NetSession
{
public:
	//-----Public Methods-----
	
	// Initialization
	NetSession();
	~NetSession();

	void							RenderDebugInfo() const;

	// Binding and Sending
	bool							Bind(unsigned short port, uint16_t portRange);
	bool							SendPacket(const NetPacket* packet);

	bool							SendMessageDirect(NetMessage* message, const NetSender_t& sender);

	// Message Definitions
	void							RegisterMessageDefinition(uint8_t messageID, const std::string& name, NetMessage_cb callback, eNetMessageOption options = NET_MSG_OPTION_NONE);
	const NetMessageDefinition_t*	GetMessageDefinition(const std::string& name);
	const NetMessageDefinition_t*	GetMessageDefinition(const uint8_t index);
	bool							GetMessageDefinitionIndex(const std::string& name, uint8_t& out_index);

	// Connections
	bool							AddConnection(uint8_t bindingIndex, NetAddress_t targetAddress);
	void							CloseAllConnections();
	NetConnection*					GetConnection(uint8_t connectionIndex) const;
	uint8_t							GetLocalConnectionIndex() const;

	// General message processing
	void							ProcessIncoming();
	void							ProcessOutgoing();

	// Network Simulation
	void							SetSimLoss(float lossAmount);
	void							SetSimLatency(float minLatency, float maxLatency);
	bool							IsReceiving() const;

	// Network tick
	void							SetNetTickRate(float hertz);
	float							GetTimeBetweenSends() const;

	// Heartbeat
	void							SetConnectionHeartbeatInterval(float hertz);
	float							GetHeartbeatInterval() const;


private:
	//-----Private Methods-----

	void							RegisterCoreMessages();

	void							ReceiveIncoming();

	void							PushNewReceive(PendingReceive& pending);
	bool							GetNextReceive(PendingReceive& out_pending);

	void							SortDefinitions();
	bool							VerifyPacket(NetPacket* packet);
	void							ProcessReceivedPacket(NetPacket* packet, const NetAddress_t& senderAddress);


private:
	//-----Private Data-----

	UDPSocket*									m_boundSocket;
	std::vector<NetConnection*>					m_connections;
	std::vector<const NetMessageDefinition_t*>	m_messageDefinitions;

	uint8_t										m_localConnectionIndex = 0xff;

	// Net sim, latency in milliseconds
	float										m_lossChance = 0.f;
	FloatRange									m_latencyRange;

	std::thread									m_receivingThread;
	std::mutex									m_receiveLock;
	std::vector<PendingReceive>					m_receiveQueue;
	bool m_isReceiving = false;

	// Network tick
	float										m_timeBetweenSends = 0.f;

	// Heartbeat
	float										m_heartBeatInverval = 0.5f;

};
