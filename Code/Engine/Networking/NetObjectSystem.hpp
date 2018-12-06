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
class NetObjectView;
class NetObjectConnectionView;

class NetObjectSystem
{
public:
	//-----Public Methods-----
	
	NetObjectSystem(NetSession* session);
	~NetObjectSystem();

	void Update();

	void RegisterNetObject(NetObject* netObj);
	NetObject* UnregisterNetObjectByNetworkID(uint16_t networkID);
	NetObject* UnregisterNetObjectByLocalObject(void* localObject);
	void RegisterNetObjectType(const NetObjectType_t& type);

	void SyncObject(uint8_t typeID, void* localObject);
	void UnsyncObject(void* localObject);

	void AddConnectionViewForIndex(uint8_t connectionIndex);
	void ClearConnectionViewForIndex(uint8_t connectionIndex);

	std::vector<NetMessage*>	GetMessagesToConstructAllNetObjects() const;
	bool						GetNextSnapshotUpdateMessage(NetMessage* out_message, uint8_t connectionIndex);

	// Accessors
	const NetObjectType_t*	GetNetObjectTypeForTypeID(uint8_t typeID) const;
	NetObject*				GetNetObjectForLocalObject(void* localObject);
	NetObject*				GetNetObjectForNetworkID(uint16_t networkID);


private:
	//-----Private Methods-----
	
	void			UpdateLocalSnapshots();
	uint16_t		GetUnusedNetworkID();

	void			AddNetObjectViewToAllConnectionViews(NetObject* netObject);
	void			RemoveNetObjectViewFromAllConnectionViews(NetObject* netObject);


private:
	//-----Private Data-----
	
	NetSession*						m_session = nullptr;
	std::vector<NetObjectType_t>	m_netObjectTypes; // By value
	std::vector<NetObject*>			m_netObjects;
	uint16_t						m_nextNetworkID = 0;

	NetObjectConnectionView*		m_connectionViews[MAX_CONNECTIONS];

};
