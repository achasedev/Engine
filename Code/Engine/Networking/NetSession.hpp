/************************************************************************/
/* File: NetSession.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single collection of network connections
/************************************************************************/
#include "Engine/Networking/NetAddress.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include <vector>
#include <string>

class UDPSocket;
class NetPacket;
class NetMessage;
class BytePacker;
class NetConnection;


class NetSession
{
public:
	//-----Public Methods-----
	
	// Initialization
	NetSession();


	// Binding and Sending
	bool		Bind(unsigned short port);
	bool		SendPacket(const NetPacket* packet);


	// Message Definitions
	void							RegisterMessageDefinition(const std::string& name, NetMessage_cb callback);
	const NetMessageDefinition_t*	GetMessageDefinition(const std::string& name);
	const NetMessageDefinition_t*	GetMessageDefinition(const uint8_t index);


	// Connections
	bool AddConnection(unsigned int bindingIndex, NetAddress_t targetAddress);
	void CloseAllConnections();


	// General message processing
	void ProcessIncoming();
	void ProcessOutgoing();


private:
	//-----Private Methods-----

	void SortDefinitions();
	void ProcessReceivedPacket(NetPacket* packet);


private:
	//-----Private Data-----

	UDPSocket* m_boundSocket;
	std::vector<NetConnection*> m_connections;
	std::vector<const NetMessageDefinition_t*> m_messageDefinitions;

};
