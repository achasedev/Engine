/************************************************************************/
/* File: NetSession.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a single collection of network connections
/************************************************************************/
#include "Engine/Networking/NetAddress.hpp"
#include <vector>
#include <map>

class UDPSocket;
class NetMessage;
class BytePacker;
class NetConnection;
struct NetSender_t;

// Callback for the NetSession
typedef bool(*NetSession_cb)(NetMessage* msg, NetConnection* sender);

// For deferred sending of messages
struct PendingMessage_t
{
	NetConnection* connection;
	NetMessage* message;
};

class NetSession
{
public:
	//-----Public Methods-----

	NetSession(size_t maxMessageSize);

	// Registers a callback given from the game to be called on messages of a given tag
	// **This is where we differentiate between message types, and can perform different tasks based on the type**
	void RegisterMessageCallback(const char* tag, NetSession_cb callback);
	
	// Adds a binding to this session to send and receive out of (not sure of the
	// use for multiple bindings, though this might have an effect on client vs. host
	// use cases)
	bool AddBinding(unsigned short port);

	// Creates a new connection using the binding given by bindingIndex
	bool AddConnection(unsigned int bindingIndex, NetAddress_t targetAddress);

	// For use by game systems
	// This allows the game to queue a message for a certain connection
	// *Optionally* if connection == nullptr, send it out all connections
	void QueueMessage(NetConnection* connection, NetMessage* msg);


	// General message processing
	void ProcessIncoming();
	void ProcessOutgoing();


private:
	//-----Private Data-----

	// Sockets that we send and receive on - all connections below
	// work through one of these bindings
	std::vector<UDPSocket*> m_bindings;

	// Connections that we talk to (essentially a bound socket + a target address)
	// All connections have one of the above bindings as their "local side" send/receive
	std::vector<NetConnection*> m_connections;

	// For limiting how much we receive from any single message
	size_t m_maxMessageSize;

	// Callbacks for processing messages
	// The game can hook whichever callbacks it likes to this list, that listens to a give tag
	std::map<std::string, NetSession_cb> m_messageCallbacks;

	// We queue up messages during the frame, and send them out after the update step
	std::vector<PendingMessage_t> m_pendingOutgoingMessages;
};
