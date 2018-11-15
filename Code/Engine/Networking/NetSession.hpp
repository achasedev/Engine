/************************************************************************/
/* File: NetSession.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single collection of network connections
/************************************************************************/
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
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
#define MAX_CONNECTIONS (32)
#define MAX_MESSAGE_DEFINITIONS (256)
#define DEFAULT_PORT_RANGE (10)
#define JOIN_TIMEOUT (10)
#define CONNECTION_LAST_RECEIVED_TIMEOUT (10)

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

	// For host join
	NET_MSG_JOIN_REQUEST,		// unreliable
	NET_MSG_JOIN_DENY,			// unreliable
	NET_MSG_JOIN_ACCEPT,		// reliable, in-order
	NET_MSG_NEW_CONNECTION,		// reliable, in-order
	NET_MSG_HOST_FINISHED_SETUP,		// reliable, in-order
	NET_MSG_CLIENT_JOIN_FINISHED,	// reliable, in-order
	NET_MSG_CORE_COUNT
};

enum eNetMessageOption : uint32_t
{
	NET_MSG_OPTION_NONE = 0,
	NET_MSG_OPTION_CONNECTIONLESS = (1 << 0),
	NET_MSG_OPTION_RELIABLE = (1 << 1),
	NET_MSG_OPTION_IN_ORDER = (1 << 2) | NET_MSG_OPTION_RELIABLE, // All in-order traffic is reliable!
};

// Callback for the NetSession
typedef bool(*NetMessage_cb)(NetMessage* msg, const NetSender_t& sender);

struct NetMessageDefinition_t
{
	NetMessageDefinition_t(uint8_t _id, const std::string& _name, NetMessage_cb _callback, eNetMessageOption _options)
		: id(_id), name(_name), callback(_callback), options(_options) {}

	bool IsReliable() const
	{
		return (options & NET_MSG_OPTION_RELIABLE) == NET_MSG_OPTION_RELIABLE;
	}

	bool IsInOrder() const
	{
		return (options & NET_MSG_OPTION_IN_ORDER) == NET_MSG_OPTION_IN_ORDER;
	}

	uint8_t				id;
	std::string			name = "";
	NetMessage_cb		callback = nullptr;
	eNetMessageOption	options;
};

struct PendingReceive
{
	float			timeStamp = 0;
	NetPacket*		packet = nullptr;
	NetAddress_t	senderAddress;
};

// Host/Join
struct NetConnectionInfo_t
{
	NetAddress_t address;
	std::string name;
	uint8_t sessionIndex;
};

enum eSessionState
{
	SESSION_DISCONNECTED = 0,	// Session can be modified
	SESSION_BOUND,				// Bound to a socket - can send and receive connectionless messages. No connections exist
	SESSION_CONNECTING,			// Attempting to connect - waiting for a response from a host
	SESSION_JOINING,			// Has established a connection, waiting final setup information/join completion
	SESSION_READY				// We are fully in the session
};

enum eSessionError
{
	SESSION_OK,
	SESSION_ERROR_USER_DISCONNECTED,

	SESSION_ERROR_JOIN_DENIED,
	SESSION_ERROR_JOIN_DENIED_NOTHOST,
	SESSION_ERROR_JOIN_DENIED_CLOSED,
	SESSION_ERROR_JOIN_DENIED_FULL
};

class NetSession
{
public:
	//-----Public Methods-----
	
	// Initialization
	NetSession();
	~NetSession();

	// Connecting
	void							Host(const std::string& myName, uint16_t port, uint16_t portRange = DEFAULT_PORT_RANGE);
	void							Join(const std::string& myName, NetConnectionInfo_t& hostInfo);
	void							ShutdownSession();

	bool							IsHosting() const;

	// Errors
	void							SetError(eSessionError error, const std::string& errorMessage);
	void							ClearError();
	eSessionError					GetLastError(std::string& out_errorMessage);

	void							Update();

	void							RenderDebugInfo() const;

	// Sending
	bool							SendPacket(const NetPacket* packet);
	bool							SendMessageDirect(NetMessage* message, const NetSender_t& sender);
	void							BroadcastMessage(NetMessage* message);

	// Message Definitions
	void							RegisterMessageDefinition(uint8_t messageID, const std::string& name, NetMessage_cb callback, eNetMessageOption options = NET_MSG_OPTION_NONE);
	const NetMessageDefinition_t*	GetMessageDefinition(const std::string& name);
	const NetMessageDefinition_t*	GetMessageDefinition(const uint8_t index);
	bool							GetMessageDefinitionIndex(const std::string& name, uint8_t& out_index);

	// Connections
	void							CloseAllConnections();
	NetConnection*					GetConnection(uint8_t connectionIndex) const;
	uint8_t							GetLocalConnectionIndex() const;

	NetConnection*					GetMyConnection() const;
	NetConnection*					GetHostConnection() const;
	bool							IsConnectionListFull() const;
	bool							DoesConnectionForAddressExist(const NetSender_t& sender) const;
	unsigned int					GetConnectionCount() const;

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

	void							TransitionToState(eSessionState state);

	bool							BindSocket(unsigned short port, uint16_t portRange);
	NetConnection*					CreateConnection(const NetConnectionInfo_t& connectionInfo);
	void							DestroyConnection(NetConnection* connection);
	void							BindConnection(uint8_t index, NetConnection* connection);
	uint8_t							GetFreeConnectionIndex() const;
	void							CheckForDisconnects();

	void							RegisterCoreMessages();

	void							ReceiveIncoming();

	void							PushNewReceive(PendingReceive& pending);
	bool							GetNextReceive(PendingReceive& out_pending);

	bool							VerifyPacket(NetPacket* packet);
	void							ProcessReceivedPacket(NetPacket* packet, const NetAddress_t& senderAddress);
	bool							ShouldMessageBeProcessed(NetMessage* message, NetConnection* connection);
	void							ProcessReceivedMessage(NetMessage* message, const NetAddress_t& address, uint8_t connectionIndex);

	// Host/Join handlers, to allow access to CreateConnection and such
	friend bool						OnJoinRequest(NetMessage* msg, const NetSender_t& sender);
	friend bool						OnJoinDeny(NetMessage* msg, const NetSender_t& sender);
	friend bool						OnJoinAccept(NetMessage* msg, const NetSender_t& sender);
	friend bool						OnNewConnection(NetMessage* msg, const NetSender_t& sender);
	friend bool						OnHostFinishedSettingClientUp(NetMessage* msg, const NetSender_t& sender);
	friend bool						OnClientFinishedTheirSetup(NetMessage* msg, const NetSender_t& sender);


private:
	//-----Private Data-----

	// State management
	eSessionState m_state = SESSION_DISCONNECTED;
	eSessionError m_error = SESSION_OK;
	std::string m_errorMesssage;

	// Convenience pointers
	NetConnection* m_myConnection = nullptr;
	NetConnection* m_hostConnection = nullptr;

	UDPSocket*									m_boundSocket;
	NetConnection*								m_boundConnections[MAX_CONNECTIONS];
	const NetMessageDefinition_t*				m_messageDefinitions[MAX_MESSAGE_DEFINITIONS];

	Stopwatch									m_joinTimer;
	Stopwatch									m_stateTimer;

	// Net sim, latency in milliseconds
	float										m_lossChance = 0.f;
	FloatRange									m_latencyRange;

	// Receiving
	std::thread									m_receivingThread;
	std::mutex									m_receiveLock;
	std::vector<PendingReceive>					m_receiveQueue;
	bool m_isReceiving = false;

	// Network tick in seconds
	float										m_timeBetweenSends = 0.f;

	// Heartbeat in seconds
	float										m_heartBeatInverval = 1.f;

};
