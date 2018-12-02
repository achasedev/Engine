/************************************************************************/
/* File: NetObject.hpp
/* Author: Andrew Chase
/* Date: December 2nd 2018
/* Description: Class for the network representation of a game object
/************************************************************************/
#pragma once

struct NetObjectType_t;

class NetObject
{
public:
	//-----Public Methods-----
	NetObject() {}
	~NetObject() {}


public:
	//-----Public Data-----
	
	const NetObjectType_t* m_netObjectType;
	uint16_t		m_networkID = 0;
	void*			m_localObjectPtr;
};
