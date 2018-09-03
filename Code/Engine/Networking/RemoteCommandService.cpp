#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Networking/RemoteCommandService.hpp"
#include "Engine/Core/Threading/Threading.hpp"

#define SERVICE_PORT 29283
#define MAX_SERVICE_CONNECTIONS 32

void HostServiceThread(void* params);


RemoteCommandService* RemoteCommandService::s_instance = nullptr;

void RemoteCommandService::Initialize()
{
	s_instance = new RemoteCommandService();
}


bool RemoteCommandService::Host()
{

}

bool RemoteCommandService::Join(const char* portAddress)
{
	NetAddress_t netAddress(portAddress, false);

	bool connected = m_socket.Connect(netAddress);

	return connected;
}

RemoteCommandService::RemoteCommandService()
	: m_state(STATE_INITIAL)
{
	bool connected = Join(Stringf("localhost:%i", SERVICE_PORT).c_str());

	if (connected)
	{
		m_state = STATE_CLIENT;
	}
	else
	{
		// Attempt to host
		Thread::CreateAndDetach(HostServiceThread);
	}
}

//-----------------------------------------------------------------------------------------------
// Function to listen for connections on a separate thread, to allow console printing
//
void HostServiceThread(void* params)
{
	UNUSED(params);
	TCPSocket hostSocket;
	if (!hostSocket.Listen(SERVICE_PORT, MAX_SERVICE_CONNECTIONS))
	{
		return;
	}

// 	while (hostSocket.IsListening() && RemoteCommandService::IsHosting())
// 	{
// 		TCPSocket* clientSocket = hostSocket.Accept();
// 	}

	// Done accepting connections, so close
	hostSocket.Close();
}