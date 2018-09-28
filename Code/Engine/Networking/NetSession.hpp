/************************************************************************/
/* File: NetSession.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single collection of network connections
/************************************************************************/
#include "Engine/Networking/NetAddress.hpp"
#include <vector>
#include <string>

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

// Callback for the NetSession
typedef bool(*NetMessage_cb)(NetMessage* msg, const NetSender_t& sender);

struct NetMessageDefinition_t
{
	NetMessageDefinition_t(const std::string& _name, NetMessage_cb _callback)
		: name(_name), callback(_callback) {}

	std::string		name = "";
	uint8_t			sessionIndex = 0;
	NetMessage_cb	callback = nullptr;
};

class NetSession
{
public:
	//-----Public Methods-----
	
	// Initialization
	NetSession();


	// Binding and Sending
	bool							Bind(unsigned short port, uint16_t portRange);
	bool							SendPacket(const NetPacket* packet);

	bool							SendMessageDirect(NetMessage* message, const NetSender_t& sender);

	// Message Definitions
	void							RegisterMessageDefinition(const std::string& name, NetMessage_cb callback);
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


private:
	//-----Private Methods-----

	void							SortDefinitions();
	bool							VerifyPacket(NetPacket* packet);
	void							ProcessReceivedPacket(NetPacket* packet, const NetAddress_t& senderAddress);


private:
	//-----Private Data-----

	UDPSocket*									m_boundSocket;
	std::vector<NetConnection*>					m_connections;
	std::vector<const NetMessageDefinition_t*>	m_messageDefinitions;

	uint8_t										m_localConnectionIndex = 0xff;

};
