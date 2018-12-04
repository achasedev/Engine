#include "Engine/Networking/NetObject.hpp"
#include "Engine/Networking/NetObjectType.hpp"
#include <stdlib.h>

NetObject::NetObject(const NetObjectType_t* type, uint16_t networkID, void* localObject, bool doIOwnObject)
	: m_netObjectType(type), m_networkID(networkID), m_localObjectPtr(localObject), m_doIOwnObject(doIOwnObject)
{
	// Allocate the snapshots
	size_t snapShotSize = m_netObjectType->snapshotSize;

	m_localSnapshot = malloc(snapShotSize);
	m_lastReceivedSnapshot = malloc(snapShotSize);
}

NetObject::~NetObject()
{
	if (m_localSnapshot != nullptr)
	{
		free(m_localSnapshot);
		m_localSnapshot = nullptr;
	}

	if (m_lastReceivedSnapshot != nullptr)
	{
		free(m_lastReceivedSnapshot);
		m_lastReceivedSnapshot = nullptr;
	}
}

const NetObjectType_t* NetObject::GetNetObjectType() const
{
	return m_netObjectType;
}

void* NetObject::GetLocalSnapshot() const
{
	return m_localSnapshot;
}

void* NetObject::GetLastReceivedSnapshot() const
{
	return m_lastReceivedSnapshot;
}

void* NetObject::GetLocalObject() const
{
	return m_localObjectPtr;
}

uint16_t NetObject::GetNetworkID() const
{
	return m_networkID;
}

bool NetObject::DoIOwn() const
{
	return m_doIOwnObject;
}

