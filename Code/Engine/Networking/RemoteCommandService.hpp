#pragma once
#include "Engine/Networking/TCPSocket.hpp"
#include <vector>

enum eServiceState
{
	STATE_INITIAL = 0,
	STATE_CLIENT,
	STATE_HOST,
	STATE_DELAY,
	NUM_STATES
};

class RemoteCommandService
{
public:
	//-----Public Methods-----

	void Initialize();
	void Update();
	void Render() const;

	bool Host();
	bool Join(const char* address);

	static bool IsHosting();

private:
	//-----Private Methods-----

	RemoteCommandService();
	~RemoteCommandService();
	RemoteCommandService(const RemoteCommandService& copy) = delete;

private:
	//-----Private Data-----

	bool m_isHosting = false;
	eServiceState m_state;
	TCPSocket m_socket;
	std::vector<TCPSocket> m_connections;

	static RemoteCommandService* s_instance;

};