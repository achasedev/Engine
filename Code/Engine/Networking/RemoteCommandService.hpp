/************************************************************************/
/* File: RemoteCommandService.hpp
/* Author: Andrew Chase
/* Date: September 20th, 2018
/* Description: Class to represent a remote dev console instance
/************************************************************************/
#pragma once
#include "Engine/Networking/TCPSocket.hpp"
#include <vector>

// Enum to control state flow
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

	// Framework
	static void Initialize();
	static void Shutdown();
	void BeginFrame();
	void Render();

	// Statics
	static bool Send(const std::string& message, int connectionIndex, bool isEcho);
	static void Join(const std::string& address);
	static void Host(unsigned short port);

	static RemoteCommandService*	GetInstance();
	static int						GetConnectionCount();


private:
	//-----Private Methods-----

	RemoteCommandService();
	~RemoteCommandService();
	RemoteCommandService(const RemoteCommandService& copy) = delete;

	void InitializeUILayout();
	static void InitializeConsoleCommands();

	// Update Step
	void Update_Initial();
	void Update_TryToJoinLocal();
	void Update_TryToJoinAddress();
	void Update_TryToHost();
	void Update_Delay();
	void Update_Host();
	void Update_Client();

	void CheckForNewConnections();
	void ProcessAllConnections();
		void ProcessConnection(int connectionIndex);
		void ProcessMessage(int connectionIndex);
	void CleanUpClosedConnections();

	void CloseAllConnections();


private:
	//-----Private Data-----

	eServiceState				m_state;
	TCPSocket					m_hostListenSocket;
	unsigned short				m_hostListenPort;

	std::vector<TCPSocket*>		m_connections;
	std::vector<BytePacker*>	m_buffers;

	Stopwatch*					m_delayTimer = nullptr;
	std::string					m_joinRequestAddress;


	// UI
	AABB2 m_bounds;

	float m_borderThickness;
	float m_textHeight;
	float m_textPadding;

	static RemoteCommandService* s_instance;

};
