/************************************************************************/
/* File: NetSequenceChannel.cpp
/* Author: Andrew Chase
/* Date: October 31st 2018
/* Description: Implementation of the NetSequenceChannel class
/************************************************************************/
#include "Engine/Networking/NetMessage.hpp"
#include "Engine/Networking/NetSequenceChannel.hpp"


//-----------------------------------------------------------------------------------------------
// Destructor
//
NetSequenceChannel::~NetSequenceChannel()
{
	ClearOutOfOrderMessages();
}


//-----------------------------------------------------------------------------------------------
// Adds the given message to the queue to be processed later, avoiding duplicates
//
void NetSequenceChannel::AddOutOfOrderMessage(NetMessage* msg)
{
	// Check for duplicates
	int numMessages = (int)m_outOfOrderMessages.size();
	uint16_t sequenceIDToFind = msg->GetSequenceID();

	for (int i = 0; i < numMessages; ++i)
	{
		if (m_outOfOrderMessages[i]->GetSequenceID() == sequenceIDToFind)
		{
			// Already exists, so return
			return;
		}
	}

	// Doesn't exist, so add it
	m_outOfOrderMessages.push_back(msg);
}


//-----------------------------------------------------------------------------------------------
// Increments and returns the id to put on the next in-order message to send
//
uint16_t NetSequenceChannel::GetAndIncrementNextIDToSend()
{
	return m_nextSequenceIDToSend++;
}


//-----------------------------------------------------------------------------------------------
// Returns the next sequential packet in the queue to process, nullptr if the next expected
// message isn't in the list
//
NetMessage* NetSequenceChannel::GetNextMessageToProcess()
{
	// Search for the next expected ID in the list
	int numMessages = (int)m_outOfOrderMessages.size();

	for (int i = 0; i < numMessages; ++i)
	{
		NetMessage* msg = m_outOfOrderMessages.at(i);
		if (msg->GetSequenceID() == m_nextSequenceIDToProcess)
		{
			m_outOfOrderMessages.erase(m_outOfOrderMessages.begin() + i);
			return msg;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Increments the next ID that we expect/will process as a receiver
//
void NetSequenceChannel::IncrementNextExpectedID()
{
	++m_nextSequenceIDToProcess;
}


//-----------------------------------------------------------------------------------------------
// Clears the out of order messages in the channel
//
void NetSequenceChannel::ClearOutOfOrderMessages()
{
	for (int i = 0; i < (int)m_outOfOrderMessages.size(); ++i)
	{
		delete m_outOfOrderMessages[i];
	}

	m_outOfOrderMessages.clear();
}


//-----------------------------------------------------------------------------------------------
// Returns true if the given ID is the ID we expect to process next
//
bool NetSequenceChannel::IsMessageNextExpected(uint16_t sequenceID) const
{
	return (m_nextSequenceIDToProcess == sequenceID);
}
