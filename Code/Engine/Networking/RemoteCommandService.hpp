#pragma once
#include "Engine/Networking/TCPSocket.hpp"
#include <vector>

enum eServiceState
{
	STATE_INITIAL = 0,
	STATE_TRYTOJOINLOCAL,
	STATE_TRYTOJOINADDRESS,
	STATE_TRYTOHOST,
	STATE_DELAY,
	STATE_HOST,
	STATE_CLIENT,
	NUM_STATES
};

class Stopwatch;
class BytePacker;

class RemoteCommandService
{
public:
	//-----Public Methods-----

	static void Initialize();
	static void Shutdown();
	void BeginFrame();

	static RemoteCommandService* GetInstance();
	static bool Send(const std::string& command, int connectionIndex);
	static int	GetConnectionCount();

private:
	//-----Private Methods-----

	RemoteCommandService();
	~RemoteCommandService();
	RemoteCommandService(const RemoteCommandService& copy) = delete;

	static void InitializeConsoleCommands();

	void Update_Initial();
	void Update_TryToJoinLocal();
	void Update_TryToJoinAddress();
	void Update_TryToHost();
	void Update_Delay();
	void Update_Host();
	void Update_Client();

	void CheckForNewConnections();
	void ProcessAllConnections();
		void ProcessConnection(TCPSocket* connection);
		void ProcessMessage(TCPSocket* connection, BytePacker* buffer);
	void CleanUpClosedConnections();

	BytePacker* GetSocketBuffer(TCPSocket* socket);
	

private:
	//-----Private Data-----

	eServiceState				m_state;
	TCPSocket					m_hostListenSocket;

	std::vector<TCPSocket*>		m_connections;
	std::vector<BytePacker*>	m_buffers;

	Stopwatch*					m_delayTimer = nullptr;
	std::string					m_joinRequestAddress;

	static RemoteCommandService* s_instance;

};