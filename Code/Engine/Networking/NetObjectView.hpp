/************************************************************************/
/* File: NetObjectView.hpp
/* Author: Andrew Chase
/* Date: December 3rd 2018
/* Description: Class to represent a connection's current state of a single
/*				object as seen by the host
/************************************************************************/
#pragma once
#include "Engine/Core/Time/Stopwatch.hpp"

class NetObject;

class NetObjectView
{
public:
	//-----Public Methods-----
	
	NetObjectView(NetObject* netObject);

	// Mutators
	void ResetTimeSinceLastSend();

	// Accessors
	float GetTimeSinceLastSend() const;
	NetObject* GetNetObject() const;

	
private:
	//-----Private Data-----
	
	NetObject* m_netObject = nullptr;
	Stopwatch m_lastSentTimer;
};
