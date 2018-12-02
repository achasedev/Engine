#include "Engine/Networking/NetObject.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Networking/NetObjectSystem.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"


//-----------------------------------------------------------------------------------------------
// Adds the given net object to the collection; this object should be one created by another session,
// and we added it through a NetObjCreate message
//
void NetObjectSystem::RegisterNetObject(NetObject* netObj)
{
	m_netObjects.push_back(netObj);
}


void NetObjectSystem::RegisterNetObjectType(const NetObjectType_t& type)
{
	// Ensure there are no duplicate types
	int typeCount = (int) m_netObjectTypes.size();

	for (int typeIndex = 0; typeIndex < typeCount; ++typeIndex)
	{
		ASSERT_OR_DIE(type.id != m_netObjectTypes[typeIndex].id, Stringf("Error: NetObjectSystem::RegisterNetObjectType() received duplicate id of %i", type.id));
	}

	m_netObjectTypes.push_back(type);
}

void NetObjectSystem::SyncObject(uint8_t typeID, void* localObject)
{
	// Create and add the object
	NetObject* netObj = new NetObject();
	netObj->m_netObjectType = *GetNetObjectTypeForTypeID(typeID);
	netObj->m_networkID = GetUnusedNetworkID();
	netObj->m_localObjectPtr = localObject;
	
	m_netObjects.push_back(netObj);

	// Send a create message
	NetMessage* createMessage = new NetMessage("netobj_create", m_session);

	// Write the NetObjectSystem-side information
	createMessage->Write(typeID);
	createMessage->Write(netObj->m_networkID);

	// Let the game write it's information
	netObj->m_netObjectType.sendCreate(*createMessage, localObject);

	// Send it out
	m_session->BroadcastMessage(createMessage);
}

const NetObjectType_t* NetObjectSystem::GetNetObjectTypeForTypeID(uint8_t typeID) const
{
	int typeCount = (int)m_netObjectTypes.size();

	for (int typeIndex = 0; typeIndex < typeCount; ++typeIndex)
	{
		if (typeID == m_netObjectTypes[typeIndex].id)
		{
			return &m_netObjectTypes[typeIndex];
		}
	}

	ERROR_AND_DIE(Stringf("Error: NetObjectSystem::GetNetObjectTypeForTypeID() couldn't find type for ID %i", typeID));
	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns a network id that isn't in use
//
uint16_t NetObjectSystem::GetUnusedNetworkID()
{
	uint16_t id = m_nextNetworkID;

	int netObjCount = (int)m_netObjects.size();

	ASSERT_OR_DIE(netObjCount <= 0xffff, Stringf("Error: NetObjectSystem::GetUnusedNetworkID() couldn't find a new ID, all in use"));

	bool done = false;

	while (!done)
	{
		bool foundDuplicate = false;

		for (int index = 0; index < netObjCount; ++index)
		{
			if (m_netObjects[index]->m_networkID == id)
			{
				// Found an object that already has this id, so go on to the next id
				// and try again
				foundDuplicate = true;
			}

			if (foundDuplicate)
			{
				id++;
				break;
			}
		}

		if (!foundDuplicate)
		{
			done = true;
		}
	}

	m_nextNetworkID = id + 1;
	return id;
}

