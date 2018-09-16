// NetSession.cpp

#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/Socket.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetConnection.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - sets the max message size only
//
NetSession::NetSession(size_t maxMessageSize)
	: m_maxMessageSize(maxMessageSize)
{
}


//-----------------------------------------------------------------------------------------------
// Adds a call back for message handling
//
void NetSession::RegisterMessageCallback(const char* tag, NetSession_cb callback)
{
	// Should replace the existing callback, or add it
	std::string key(tag);

	m_messageCallbacks[key] = callback;
}


//-----------------------------------------------------------------------------------------------
// Adds a socket binding for sending and listening to connections
//
bool NetSession::AddBinding(unsigned short port)
{
	UDPSocket* newSocket = new UDPSocket();

	NetAddress_t localAddress;
	bool foundLocal = NetAddress_t::GetLocalAddress(&localAddress, port, true);

	if (!foundLocal)
	{
		ConsoleErrorf("Couldn't find local address to bind to");
		return false;
	}

	bool bound = newSocket->bind(localAddress);

	if (bound)
	{
		m_bindings.push_back(newSocket);
		ConsolePrintf(Rgba::GREEN, "NetSession bound to port %u", port);
	}
	else
	{
		ConsoleErrorf("Error: NetSession couldn't bind to port %u", port);
	}

	return bound;
}


//-----------------------------------------------------------------------------------------------
// Create and add a new connection
//
bool NetSession::AddConnection(unsigned int bindingIndex, NetAddress_t targetAddress)
{
	if (bindingIndex >= (unsigned int)m_bindings.size())
	{
		ConsoleErrorf("Error: NetSession::AddConnection() received bindingIndex out of range, %i", bindingIndex);
		return;
	}

	NetConnection* newConnection = new NetConnection(m_bindings[bindingIndex], targetAddress);
	m_connections.push_back(newConnection);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Queues a message to be sent at the end of the update step
//
void NetSession::QueueMessage(NetConnection* connection, NetMessage* msg)
{
	PendingMessage_t pendingMessage;
	pendingMessage.message = msg;
	pendingMessage.connection = connection;

	m_pendingOutgoingMessages.push_back(pendingMessage);
}


//-----------------------------------------------------------------------------------------------
// Receives on all current connections, calling callbacks on all receiving messages depending on their tag
//
void NetSession::ProcessIncoming()
{
	// Receive on each connection, calling the callbacks based on tag
	for (int connectionIndex = 0; connectionIndex < (int) m_connections.size(); ++connectionIndex)
	{
		NetMessage* message = new NetMessage();
		int amountReceived = m_connections[connectionIndex]->Receive(message, m_maxMessageSize);

		if (amountReceived > 0)
		{
			// Check the tag to see what we should do with it
			std::string tag;
			message->ReadString(tag); // Messages always start off with a string for their tag

			if (tag.size() > 0)
			{
				// Check if the tag has a callback
				bool tagHasCallback = m_messageCallbacks.find(tag) != m_messageCallbacks.end();

				if (tagHasCallback)
				{
					m_messageCallbacks[tag](message, m_connections[connectionIndex]);
				}
			}
		}

		// Done with the message
		delete message;
	}
}


//-----------------------------------------------------------------------------------------------
// Send out all pending messages
//
void NetSession::ProcessOutgoing()
{
	// Send all pending messages out

	for (int i = (int) m_pendingOutgoingMessages.size() - 1; i >= 0; ++i)
	{
		PendingMessage_t currMessage = m_pendingOutgoingMessages[i];
		currMessage.connection->Send(currMessage.message);

		// Done with the message
		delete currMessage.message;
	}

	m_pendingOutgoingMessages.clear();
}
