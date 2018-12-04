/************************************************************************/
/* File: NetObject.hpp
/* Author: Andrew Chase
/* Date: December 2nd 2018
/* Description: Class for the network representation of a game object
/************************************************************************/
#pragma once
#include <stdint.h.>

struct NetObjectType_t;

class NetObject
{
public:
	//-----Public Methods-----

	NetObject(const NetObjectType_t* type, uint16_t networkID, void* localObject, bool doIOwnObject);
	~NetObject();

	const NetObjectType_t*	GetNetObjectType() const;
	void*					GetLocalSnapshot() const;
	void*					GetLastReceivedSnapshot() const;
	void*					GetLocalObject() const;
	uint16_t				GetNetworkID() const;
	bool					DoIOwn() const;


private:
	//-----Private Data-----
	
	bool					m_doIOwnObject = false;
	const NetObjectType_t*	m_netObjectType;
	uint16_t				m_networkID = 0;
	void*					m_localObjectPtr;

	void*					m_localSnapshot = nullptr;
	void*					m_lastReceivedSnapshot = nullptr;

};
