/************************************************************************/
/* File: NetSequenceChannel.hpp
/* Author: Andrew Chase
/* Date: October 31st 2018
/* Description: Class to represent a queue of in-order messages
/************************************************************************/
#pragma once
#include <stdint.h>
#include <vector>

class NetMessage;
class NetSequenceChannel
{
public:
	//-----Public Methods-----

	// Mutators/Accessors
	void		AddOutOfOrderMessage(NetMessage* msg);
	uint16_t	GetAndIncrementNextIDToSend();
	NetMessage* GetNextMessageToProcess();
	void		IncrementNextExpectedID();


	// Producers
	bool		IsMessageNextExpected(uint16_t sequenceID) const;
	

private:
	//-----Private Data-----
	
	uint16_t m_nextSequenceIDToSend = 0;
	uint16_t m_nextSequenceIDToProcess = 0;
	std::vector<NetMessage*> m_outOfOrderMessages;

};
