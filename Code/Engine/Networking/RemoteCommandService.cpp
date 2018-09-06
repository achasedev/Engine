#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/Time/Stopwatch.hpp"
#include "Engine/Networking/BytePacker.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Core/Threading/Threading.hpp"
#include "Engine/Networking/RemoteCommandService.hpp"
#include "Engine/Core/DeveloperConsole/Command.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"

#define SERVICE_PORT 29283
#define MAX_CLIENTS 32
#define DELAY_TIME 5

void Command_RemoteCommand(Command& cmd);
void Command_RemoteCommandBroadcast(Command& cmd);

RemoteCommandService* RemoteCommandService::s_instance = nullptr;

void RemoteCommandService::Initialize()
{
	s_instance = new RemoteCommandService();

	InitializeConsoleCommands();
}


void RemoteCommandService::Shutdown()
{
	delete s_instance;
	s_instance = nullptr;
}

void RemoteCommandService::BeginFrame()
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


RemoteCommandService* RemoteCommandService::GetInstance()
{
	return s_instance;
}


bool RemoteCommandService::Send(const std::string& command, int connectionIndex)
{
	if (connectionIndex >= (int)s_instance->m_connections.size() || command.size() == 0)
	{
		return false;
	}

	// Make the message
	BytePacker sendPack(BIG_ENDIAN);


	bool isEcho = false;
	sendPack.WriteBytes(1, &isEcho);
	sendPack.WriteString(command);

	uint16_t messageLength = sendPack.GetWrittenByteCount();
	uint16_t msgBigEndian = messageLength;
	ToEndianness(2, &msgBigEndian, BIG_ENDIAN);

	s_instance->m_connections[connectionIndex]->Send(&msgBigEndian, 2);
	int amountSent = s_instance->m_connections[connectionIndex]->Send(sendPack.GetBuffer(), messageLength);

	if (amountSent > 0)
	{
		LogTaggedPrintf("RCS", "Sent command %s to connection index %i", command.c_str(), connectionIndex);
	}
	else
	{
		LogTaggedPrintf("RCS", "Failed to send command %s to connection index %i", command.c_str(), connectionIndex);
	}

	return (amountSent > 0);
}

int RemoteCommandService::GetConnectionCount()
{
	return (int)s_instance->m_connections.size();
}

RemoteCommandService::RemoteCommandService()
	: m_state(STATE_INITIAL)
{
	m_hostListenSocket.SetBlocking(false);
	m_delayTimer = new Stopwatch(nullptr);

	ConsolePrintf("RCS re-entered initial state");
	LogTaggedPrintf("RCS", "Entered Initial State");
}

void RemoteCommandService::InitializeConsoleCommands()
{
	Command::Register("rc", "Sends a command to a remote connection to execute.", Command_RemoteCommand);
	Command::Register("rcb", "Broadcasts a command to all remote connections.", Command_RemoteCommandBroadcast);
}

RemoteCommandService::~RemoteCommandService()
{
	for (int i = 0; i < (int)m_connections.size(); ++i)
	{
		m_connections[i]->Close();
		delete m_connections[i];
	}

	m_connections.clear();

	for (int j = 0; j < (int)m_buffers.size(); ++j)
	{
		delete m_buffers[j];
	}

	m_buffers.clear();
}

void RemoteCommandService::Update_Initial()
{
	// Ensure we're no longer hosting
	m_hostListenSocket.Close();

	// Close all existing connections
	for (int index = 0; index < (int)m_connections.size(); ++index)
	{
		m_connections[index]->Close();
		delete m_connections[index];
	}

	m_connections.clear();


	if (m_joinRequestAddress.size() > 0)
	{
		ConsolePrintf("RCS is trying to join address %s...", m_joinRequestAddress.c_str());
		LogTaggedPrintf("RCS", "Entered TryToJoinAddress State");
		m_state = STATE_TRYTOJOINADDRESS;
	}
	else
	{
		ConsolePrintf("RCS is trying to join local...");
		LogTaggedPrintf("RCS", "Entered TryToJoinLocal State");
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
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
		return;
	}

	TCPSocket* joinSocket = new TCPSocket();
	joinSocket->SetBlocking(true);

	bool connected = joinSocket->Connect(localAddress);

	if (!connected)
	{
		m_state = STATE_TRYTOHOST;
		ConsolePrintf("RCS failed to join local, trying to host...");
		LogTaggedPrintf("RCS", "Entered TryToHost State");
		return;
	}

	// Connected successfully, store off socket and go to client state
	joinSocket->SetBlocking(false);
	m_connections.push_back(joinSocket);
	m_buffers.push_back(new BytePacker(BIG_ENDIAN));
	m_state = STATE_CLIENT;
	LogTaggedPrintf("RCS", "Entered Client State");
	ConsolePrintf(Rgba::GREEN, "RCS is a client.");
}

void RemoteCommandService::Update_TryToJoinAddress()
{
	NetAddress_t netAddress(m_joinRequestAddress.c_str());

	TCPSocket* joinSocket = new TCPSocket();
	joinSocket->SetBlocking(true);
	bool connected = joinSocket->Connect(netAddress);

	if (!connected)
	{
		delete joinSocket;
		m_state = STATE_INITIAL;
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
		return;
	}

	joinSocket->SetBlocking(false);
	m_connections.push_back(joinSocket);
	m_buffers.push_back(new BytePacker(BIG_ENDIAN));
	m_state = STATE_CLIENT;
	ConsolePrintf(Rgba::GREEN, "RCS successfully joined address %s", m_joinRequestAddress.c_str());
	LogTaggedPrintf("RCS", "Entered Client State");

	// Always clear the join request
	m_joinRequestAddress.clear();
}

void RemoteCommandService::Update_TryToHost()
{
	bool isListening = m_hostListenSocket.Listen(SERVICE_PORT, MAX_CLIENTS);

	if (!isListening)
	{
		m_delayTimer->SetInterval(DELAY_TIME);
		m_state = STATE_DELAY;
		LogTaggedPrintf("RCS", "Entered Delay State");
	}
	else
	{
		m_state = STATE_HOST;
		ConsolePrintf(Rgba::GREEN, "RCS is now hosting.");
		LogTaggedPrintf("RCS", "Entered Host State");
	}
}

void RemoteCommandService::Update_Delay()
{
	if (m_delayTimer->HasIntervalElapsed())
	{
		m_delayTimer->Reset();
		m_state = STATE_INITIAL;
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
	}
}

void RemoteCommandService::Update_Host()
{
	// Check if a join request is available
	if (m_joinRequestAddress.size() > 0)
	{
		m_state = STATE_INITIAL;
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
	}

	CheckForNewConnections();
	ProcessAllConnections();
	CleanUpClosedConnections();
}

void RemoteCommandService::Update_Client()
{
	// Check if a join request is available
	if (m_joinRequestAddress.size() > 0)
	{
		m_state = STATE_INITIAL;
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
		return;
	}

	ProcessAllConnections();
	CleanUpClosedConnections();

	// No longer connected to the host, so we reset
	if (m_connections.size() == 0)
	{
		m_state = STATE_INITIAL;
		ConsolePrintf("RCS re-entered initial state");
		LogTaggedPrintf("RCS", "Entered Initial State");
	}
}

void RemoteCommandService::CheckForNewConnections()
{
	m_hostListenSocket.SetBlocking(false);
	TCPSocket* socket = m_hostListenSocket.Accept();

	if (socket != nullptr)
	{
		m_connections.push_back(socket);
		m_buffers.push_back(new BytePacker(BIG_ENDIAN));
	}
}

void RemoteCommandService::ProcessAllConnections()
{
	for (int i = 0; i < (int)m_connections.size(); ++i)
	{
		ProcessConnection(m_connections[i]);
	}
}

void RemoteCommandService::ProcessConnection(TCPSocket* connection)
{
	BytePacker *buffer = GetSocketBuffer(connection);
	buffer->Reserve(2);

	// Need to get the message length still
	if (buffer->GetWrittenByteCount() < 2)
	{
		int amountReceived = connection->Receive(buffer->GetWriteHead(), 2 - buffer->GetWrittenByteCount());
		buffer->AdvanceWriteHead(amountReceived);
	}

	bool isReadyToProcess = false;
	if (buffer->GetWrittenByteCount() >= 2)
	{
		uint16_t len;
		buffer->Peek(&len, sizeof(len));

		// Reserve enough for the size 
		buffer->Reserve(len + 2U);

		size_t bytesNeeded = len + 2U - buffer->GetWrittenByteCount();

		// If we still need more of the message
		if (bytesNeeded > 0)
		{
			size_t read = connection->Receive(buffer->GetWriteHead(), bytesNeeded);
			buffer->AdvanceWriteHead(read);

			bytesNeeded -= read;
		}

		isReadyToProcess = (bytesNeeded == 0);
	}

	if (isReadyToProcess)
	{
		buffer->AdvanceReadHead(2U);
		ProcessMessage(connection, buffer);

		// Clean up to be used
		buffer->ResetWrite();
	}
}

void RemoteCommandService::ProcessMessage(TCPSocket* connection, BytePacker* buffer)
{
	bool isEcho;
	buffer->ReadBytes(&isEcho, 1);;

	std::string str;
	if (buffer->ReadString(str))
	{
		// Succeeded in getting a command string
		if (isEcho)
		{
			// Print to console, with info
			NetAddress_t address = connection->GetNetAddress();
			ConsolePrintf("[%s]: %s", address.ToString().c_str(), str.c_str());
		}
		else
		{
			Command::Run(str);
		}
	}
}

void RemoteCommandService::CleanUpClosedConnections()
{
	for (int i = (int) m_connections.size() - 1; i >= 0; --i)
	{
		if (m_connections[i]->IsClosed())
		{
			m_connections.erase(m_connections.begin() + i);
			
			// Free up the byte packer for this connection
			delete m_buffers[i];
			m_buffers.erase(m_buffers.begin() + i);
		}
	}
}

BytePacker* RemoteCommandService::GetSocketBuffer(TCPSocket* socket)
{
	for (int i = 0; i < m_connections.size(); ++i)
	{
		if (m_connections[i] == socket)
		{
			return m_buffers[i];
		}
	}

	return nullptr;
}

void Command_RemoteCommand(Command& cmd)
{
	std::string commandToExecute;
	cmd.GetParam("c", commandToExecute);

	if (commandToExecute.size() == 0)
	{
		ConsoleErrorf("No command specified for remote command");
		return;
	}

	int connectionIndex = 0;
	cmd.GetParam("i", connectionIndex, &connectionIndex);

	bool sent = RemoteCommandService::Send(commandToExecute, connectionIndex);

	if (sent)
	{
		ConsolePrintf(Rgba::GREEN, "Command \"%s\" sent to connection %i", commandToExecute.c_str(), connectionIndex);
	}
	else
	{
		ConsoleErrorf("Couldn't sent command %s to connection %i", commandToExecute.c_str(), connectionIndex);
	}
}

void Command_RemoteCommandBroadcast(Command& cmd)
{
	std::string commandToExecute;
	cmd.GetParam("c", commandToExecute);

	if (commandToExecute.size() == 0)
	{
		ConsoleErrorf("No command specified for remote command");
		return;
	}

	int connectionCount = RemoteCommandService::GetConnectionCount();
	for (int connectionIndex = 0; connectionIndex < connectionCount; ++connectionIndex)
	{
		bool sent = RemoteCommandService::Send(commandToExecute, connectionIndex);

		if (sent)
		{
			ConsolePrintf(Rgba::GREEN, "Command \"%s\" sent to connection %i", commandToExecute.c_str(), connectionIndex);
		}
		else
		{
			ConsoleErrorf("Couldn't sent command %s to connection %i", commandToExecute.c_str(), connectionIndex);
		}
	}
}