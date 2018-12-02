/************************************************************************/
/* File: NetObjectSystem.hpp
/* Author: Andrew Chase
/* Date: December 2nd, 2018
/* Description: Class to create, destroy, and update net objects in a
/*				NetSession instance
/************************************************************************/
#pragma once
#include <vector>
#include <stdint.h>
#include "Engine/Networking/NetObjectType.hpp"

class NetObject;
class NetSession;

class NetObjectSystem
{
public:
	//-----Public Methods-----
	
	void RegisterNetObject(NetObject* netObj);
	void RegisterNetObjectType(const NetObjectType_t& type);

	void SyncObject(uint8_t typeID, void* localObject);
	void UnsyncObject(void* localObject);

	// Accessors
	const NetObjectType_t* GetNetObjectTypeForTypeID(uint8_t typeID) const;


private:
	//-----Private Methods-----
	
	NetObject*		GetNetObjectForLocalObject(void* localObject);
	NetObject*		GetNetObjectForNetworkID(uint16_t id);

	uint16_t		GetUnusedNetworkID();

	
private:
	//-----Private Data-----
	
	NetSession*						m_session = nullptr;
	std::vector<NetObjectType_t>	m_netObjectTypes; // By value
	std::vector<NetObject*>			m_netObjects;
	uint16_t						m_nextNetworkID = 0;
};
