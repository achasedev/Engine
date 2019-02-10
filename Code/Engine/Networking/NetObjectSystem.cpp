#include "Engine/Networking/NetObject.hpp"
#include "Engine/Networking/NetSession.hpp"
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetObjectView.hpp"
#include "Engine/Core/Utility/StringUtils.hpp"
#include "Engine/Networking/NetObjectSystem.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"
#include "Engine/Networking/NetObjectConnectionView.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor
//
NetObjectSystem::NetObjectSystem(NetSession* session)
	: m_session(session)
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		m_connectionViews[i] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetObjectSystem::~NetObjectSystem()
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_connectionViews[i] != nullptr)
		{
			delete m_connectionViews[i];
			m_connectionViews[i] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Update
//
void NetObjectSystem::Update()
{
	UpdateLocalSnapshots();
}


//-----------------------------------------------------------------------------------------------
// Adds the given net object to the collection; this object should be one created by another session,
// and we added it through a NetObjCreate message
//
void NetObjectSystem::RegisterNetObject(NetObject* netObj)
{
	ASSERT_OR_DIE(!netObj->DoIOwn(), "Error: NetObjectSystem::RegisterNetObject() tried to register an object we own.");
	m_netObjects.push_back(netObj);
}


//-----------------------------------------------------------------------------------------------
// Removes the net object from the system and returns it; used for when this session would receive 
// a destroy message from a host
//
NetObject* NetObjectSystem::UnregisterNetObjectByNetworkID(uint16_t networkID)
{
	int objCount = (int)m_netObjects.size();

	NetObject* netObject = nullptr;
	for (int objIndex = 0; objIndex < objCount; ++objIndex)
	{
		if (m_netObjects[objIndex]->GetNetworkID() == networkID)
		{
			netObject = m_netObjects[objIndex];
			m_netObjects.erase(m_netObjects.begin() + objIndex);
			break;
		}
	}

	if (netObject != nullptr)
	{
		RemoveNetObjectViewFromAllConnectionViews(netObject);
	}

	return netObject;
}

NetObject* NetObjectSystem::UnregisterNetObjectByLocalObject(void* localObject)
{
	NetObject* netObject = GetNetObjectForLocalObject(localObject);
	return UnregisterNetObjectByNetworkID(netObject->GetNetworkID());
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
	uint16_t networkID = GetUnusedNetworkID();
	const NetObjectType_t* type = GetNetObjectTypeForTypeID(typeID);
	ASSERT_OR_DIE(type != nullptr, "Error: NetObjectSystem::SyncObject() couldn't find type.");

	// Create and add the object
	NetObject* netObj = new NetObject(type, networkID, localObject, true);
	m_netObjects.push_back(netObj);

	// Add it to all connection views
	AddNetObjectViewToAllConnectionViews(netObj);

	// Send a create message
	NetMessage* createMessage = new NetMessage("netobj_create", m_session);

	// Write the NetObjectSystem-side information
	createMessage->Write(typeID);
	createMessage->Write(networkID);

	// Let the game write it's information
	type->writeCreate(*createMessage, localObject);

	// Send it out to all connections
	m_session->BroadcastMessage(createMessage);
}

void NetObjectSystem::UnsyncObject(void* localObject)
{
	// Get the NetObject for it
	int objCount = (int)m_netObjects.size();

	NetObject* netObject = nullptr;
	for (int objIndex = 0; objIndex < objCount; ++objIndex)
	{
		if (m_netObjects[objIndex]->GetLocalObject() == localObject)
		{
			netObject = m_netObjects[objIndex];
			m_netObjects.erase(m_netObjects.begin() + objIndex);
			break;
		}
	}

	ASSERT_OR_DIE(netObject != nullptr, "Error: NetObjectSystem::UnsyncObject() couldn't find object.");
	ASSERT_OR_DIE(netObject->DoIOwn(), "Error: NetObjectSystem::UnsyncObject() tried to unsync object it doens't own.");

	// Remove it from our views
	RemoveNetObjectViewFromAllConnectionViews(netObject);

	NetMessage* destroyMessage = new NetMessage("netobj_destroy", m_session);

	// Write the network ID
	destroyMessage->Write(netObject->GetNetworkID());

	// Let the game write any destroy information (most of the time is empty)
	netObject->GetNetObjectType()->writeDestroy(*destroyMessage, localObject);

	// Broadcast it
	m_session->BroadcastMessage(destroyMessage);
}

void NetObjectSystem::AddConnectionViewForIndex(uint8_t connectionIndex)
{
	ASSERT_OR_DIE(connectionIndex != INVALID_CONNECTION_INDEX && connectionIndex < MAX_CONNECTIONS, "Error: NetObjectSystem::AddConnectionViewForIndex() received bad connection index");
	ASSERT_OR_DIE(m_connectionViews[connectionIndex] == nullptr, "Error: NetObjectSystem::AddConnectionViewForIndex() tried to add duplicate connection view.");

	NetObjectConnectionView* connView = new NetObjectConnectionView();

	// Add in all current net objects in the system
	for (int i = 0; i < (int)m_netObjects.size(); ++i)
	{
		connView->AddNetObjectView(m_netObjects[i]);
	}

	m_connectionViews[connectionIndex] = connView;
}

void NetObjectSystem::ClearConnectionViewForIndex(uint8_t connectionIndex)
{
	ASSERT_OR_DIE(connectionIndex != INVALID_CONNECTION_INDEX && connectionIndex < MAX_CONNECTIONS, "Error: NetObjectSystem::ClearConnectionViewForIndex() received bad connection index");
	ASSERT_OR_DIE(m_connectionViews[connectionIndex] != nullptr, "Error: NetObjectSystem::ClearConnectionViewForIndex() tried to clear null connection view.");
	
	delete m_connectionViews[connectionIndex];
	m_connectionViews[connectionIndex] = nullptr;
}

std::vector<NetMessage*> NetObjectSystem::GetMessagesToConstructAllNetObjects() const
{
	std::vector<NetMessage*> messages;

	int objCount = (int)m_netObjects.size();

	for (int objIndex = 0; objIndex < objCount; ++objIndex)
	{
		const NetObject* obj = m_netObjects[objIndex];

		NetMessage* msg = new NetMessage("netobj_create", m_session);

		msg->Write(obj->GetNetObjectType()->id);
		msg->Write(obj->GetNetworkID());

		obj->GetNetObjectType()->writeCreate(*msg, obj->GetLocalObject());

		messages.push_back(msg);
	}

	return messages;
}


//-----------------------------------------------------------------------------------------------
// Creates and returns the next snapshot message to send out, based on age
// Returns false if a message couldn't be created
//
bool NetObjectSystem::GetNextSnapshotUpdateMessage(NetMessage* out_message, uint8_t connectionIndex)
{
	int netObjectCount = (int)m_netObjects.size();

	if (netObjectCount == 0)
	{
		return false;
	}

	// Find the NetObjectView with minimum age
	NetObjectConnectionView* connectionView = m_connectionViews[connectionIndex];
	ASSERT_OR_DIE(connectionView != nullptr, "Error: NetObjectSystem::GetNextSnapshotUpdateMessage() had null ConnectionView for current connection.");

	NetObjectView* objectView = connectionView->GetNextObjectViewToSendUpdateFor();

	// Returns null if all the views have an elapsed time of 0.f, meaning they're up-to-date already
	if (objectView == nullptr)
	{
		return false;
	}

	NetObject* netObject = objectView->GetNetObject();
	const NetObjectType_t* type = netObject->GetNetObjectType();

	// Write the network ID
	out_message->Write(netObject->GetNetworkID());

	// Write the snapshot
	type->writeSnapshot(*out_message, netObject->GetLocalSnapshot());
	objectView->ResetTimeSinceLastSend();

	return true;
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
}


//-----------------------------------------------------------------------------------------------
// Returns the NetObject associated with the given local object, if one exists, nullptr otherwise
//
NetObject* NetObjectSystem::GetNetObjectForLocalObject(void* localObject)
{
	int netObjCount = (int)m_netObjects.size();

	for (int i = 0; i < netObjCount; ++i)
	{
		if (m_netObjects[i]->GetLocalObject() == localObject)
		{
			return m_netObjects[i];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the NetObject corresponding to the Network ID given
//
NetObject* NetObjectSystem::GetNetObjectForNetworkID(uint16_t networkID)
{
	int netObjCount = (int)m_netObjects.size();

	for (int i = 0; i < netObjCount; ++i)
	{
		if (m_netObjects[i]->GetNetworkID() == networkID)
		{
			return m_netObjects[i];
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Creates and updates all local snapshots for each NetObject
// Done for all objects regardless if we own them, so we have our current stored somewhere (even
// though we may never use it)
//
void NetObjectSystem::UpdateLocalSnapshots()
{
	int netObjCount = (int)m_netObjects.size();

	for (int i = 0; i < netObjCount; ++i)
	{
		const NetObjectType_t* type = m_netObjects[i]->GetNetObjectType();
		type->makeSnapshot(m_netObjects[i]->GetLocalSnapshot(), m_netObjects[i]->GetLocalObject());
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a network id that isn't in use
//
uint16_t NetObjectSystem::GetUnusedNetworkID()
{
	uint16_t nextIDToUse = m_nextNetworkID;

	int netObjCount = (int)m_netObjects.size();

	ASSERT_OR_DIE(netObjCount <= 0xffff, Stringf("Error: NetObjectSystem::GetUnusedNetworkID() couldn't find a new ID, all in use"));

	bool done = false;

	while (!done)
	{
		bool foundDuplicate = false;

		for (int index = 0; index < netObjCount; ++index)
		{
			if (m_netObjects[index]->GetNetworkID() == nextIDToUse)
			{
				// Found an object that already has this id, so go on to the next id
				// and try again
				foundDuplicate = true;
			}

			if (foundDuplicate)
			{
				nextIDToUse++;
				break;
			}
		}

		if (!foundDuplicate)
		{
			done = true;
		}
	}

	m_nextNetworkID = nextIDToUse + 1;
	return nextIDToUse;
}


//-----------------------------------------------------------------------------------------------
// Adds a view for the given NetObject to all NetConnectionViews
//
void NetObjectSystem::AddNetObjectViewToAllConnectionViews(NetObject* netObject)
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_connectionViews[i] != nullptr)
		{
			m_connectionViews[i]->AddNetObjectView(netObject);
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Removes the view for the current NetObject on all connection views
//
void NetObjectSystem::RemoveNetObjectViewFromAllConnectionViews(NetObject* netObject)
{
	for (int i = 0; i < MAX_CONNECTIONS; ++i)
	{
		if (m_connectionViews[i] != nullptr)
		{
			m_connectionViews[i]->RemoveNetObjectView(netObject);
		}
	}
}

