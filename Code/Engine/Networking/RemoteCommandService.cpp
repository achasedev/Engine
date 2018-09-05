#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Networking/BytePacker.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/Networking/RemoteCommandService.hpp"

#define SERVICE_PORT 29283
#define MAX_CLIENTS 32
#define DELAY_TIME 5

RemoteCommandService* RemoteCommandService::s_instance = nullptr;

void RemoteCommandService::Initialize()
{
	s_instance = new RemoteCommandService();
}


void RemoteCommandService::Update()
{
	switch (m_state)
	{
	case STATE_INITIAL:				Update_Initial();			break;
	case STATE_TRYTOJOINLOCAL:		Update_TryToJoinLocal();	break;
	case STATE_TRYTOJOINADDRESS:	Update_TryToJoinAddress();	break;
	case STATE_TRYTOHOST:			Update_TryToHost();			break;
	case STATE_DELAY:				Update_Delay();				break;
	case STATE_HOST:				Update_Host();				break;
	case STATE_CLIENT:				Update_Client();			break;
	default:
		break;
	}
}


RemoteCommandService::RemoteCommandService()
	: m_state(STATE_INITIAL)
{
	m_hostListenSocket.SetBlocking(false);

	m_delayTimer = new Stopwatch(nullptr);
}

void RemoteCommandService::Update_Initial()
{
	// Ensure we're no longer hosting
	m_hostListenSocket.Close();

	// Close all existing connections
	for (int index = 0; index < (int)m_connections.size(); ++index)
	{
		m_connections[index].Close();
	}

	m_connections.clear();


	if (m_joinRequestAddress.size() > 0)
	{
		m_state = STATE_TRYTOJOINADDRESS;
	}
	else
	{
		m_state = STATE_TRYTOJOINLOCAL;
	}
}

void RemoteCommandService::Update_TryToJoinLocal()
{
	NetAddress_t localAddress;
	bool localAddressFound = NetAddress_t::GetLocalAddress(&localAddress, SERVICE_PORT, false);

	if (!localAddressFound)
	{
		m_state = STATE_INITIAL;
		return;
	}

	TCPSocket joinSocket(false);
	bool connected = joinSocket.Connect(localAddress);

	if (!connected)
	{
		m_state = STATE_INITIAL;
		return;
	}

	// Connected successfully, store off socket and go to client state
	m_connections.push_back(joinSocket);
	m_state = STATE_CLIENT;
}

void RemoteCommandService::Update_TryToJoinAddress()
{
	NetAddress_t netAddress(m_joinRequestAddress.c_str());

	TCPSocket joinSocket(false);
	bool connected = joinSocket.Connect(netAddress);

	if (!connected)
	{
		m_state = STATE_INITIAL;
		return;
	}

	m_connections.push_back(joinSocket);
	m_state = STATE_CLIENT;
}

void RemoteCommandService::Update_TryToHost()
{
	bool isListening = m_hostListenSocket.Listen(SERVICE_PORT, MAX_CLIENTS);

	if (!isListening)
	{
		m_delayTimer->SetInterval(DELAY_TIME);
		m_state = STATE_DELAY;
	}
	else
	{
		m_state = STATE_HOST;
	}
}

void RemoteCommandService::Update_Delay()
{
	if (m_delayTimer->HasIntervalElapsed())
	{
		m_delayTimer->Reset();
		m_state = STATE_INITIAL;
	}
}

void RemoteCommandService::Update_Host()
{
	CheckForNewConnections();
	ProcessAllConnections();
	CleanUpClosedConnections();
}

void RemoteCommandService::Update_Client()
{
	ProcessAllConnections();
	CleanUpClosedConnections();
}

void RemoteCommandService::CheckForNewConnections()
{
	TCPSocket* socket = m_hostListenSocket.Accept();

	if (socket != nullptr)
	{
		m_connections.push_back(*socket);
		delete socket;
	}
}

void RemoteCommandService::ProcessAllConnections()
{
	for (int i = 0; i < (int)m_connections.size(); ++i)
	{
		ProcessConnection(&m_connections[i]);
	}
}

void RemoteCommandService::ProcessConnection(TCPSocket* connection)
{
	BytePacker *buffer = GetSocketBuffer(connection);

	// Need to get the message length still
	if (buffer->GetWrittenByteCount() < 2)
	{
		connection->Receive(buffer->GetWriteHead(), 2 - buffer->GetWrittenByteCount());
		buffer->AdvanceWriteHead(read);
	}


	bool isReadyToProcess = false;
	if (buffer->GetWrittenByteCount() >= 2)
	{
		uint16_t len;
		buffer->Peak(&len);

		uint bytesNeeded = len + 2U - buffer->GetWrittenByteCount();

		if (bytesNeeded > 0)
		{
			size_t read = tcp->Receive(buffer->GetWriteHead(), bytesNeeded);
			tcp->AdvanceWriteHead(read);

			bytesNeeded -= read;
		}

		isReadyToProcess = (bytesNeeded == 0);
	}

	if (isReadyToProcess)
	{
		buffer->AdvanceReadHead(2U);
		ProcessMessage(tcp, buffer);
		buffer->ResetWrite();
	}
}

BytePacker* RemoteCommandService::GetSocketBuffer(TCPSocket* socket)
{
	for (int i = 0; i < m_connections.size(); ++i)
	{
		if (&m_connections[i] == socket)
		{
			return m_buffers[i];
		}
	}

	return nullptr;
}
