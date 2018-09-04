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

class RemoteCommandService
{
public:
	//-----Public Methods-----

	void Initialize();
	void Update();
	void Render() const;

	static bool IsHosting();

private:
	//-----Private Methods-----

	RemoteCommandService();
	~RemoteCommandService();
	RemoteCommandService(const RemoteCommandService& copy) = delete;

	void Update_Initial();
	void Update_TryToJoinLocal();
	void Update_TryToJoinAddress();
	void Update_TryToHost();
	void Update_Delay();
	void Update_Host();
	void Update_Client();

	void CheckForNewConnections();
	void ProcessConnections();
		void ProcessMessage();
	void CleanUpClosedConnections();

private:
	//-----Private Data-----

	eServiceState			m_state;
	TCPSocket				m_hostListenSocket;
	std::vector<TCPSocket>	m_connections;

	Stopwatch*				m_delayTimer = nullptr;
	std::string				m_joinRequestAddress;

	static RemoteCommandService* s_instance;

};