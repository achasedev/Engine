/************************************************************************/
/* File: NetObjectType.hpp
/* Author: Andrew Chase
/* Date: December 2nd 2018
/* Description: Header file for a Network Object type definition
/************************************************************************/
#pragma once
#include <stdint.h>

class NetMessage;

typedef void(*NetObjectSendCreate)(NetMessage& msg, void* args);
typedef void*(*NetObjectReceiveCreate)(NetMessage& msg);
typedef void(*NetObjectSendDestroy)(NetMessage& msg, void* args);
typedef void(*NetObjectReceiveDestroy)(NetMessage* msg);

typedef void(*NetObjectMakeSnapshot)(void* snapshot, const void* object);
typedef void(*NetObjectSendSnapshot)(NetMessage& msg, const void* snapshot);
typedef void(*NetObjectReceiveSnapshot)(NetMessage& msg, void* out_snapshot);
typedef void(*NetObjectApplySnapshot)(void* snapshot, void* object);

struct NetObjectType_t
{
	// ID
	uint8_t	id;

	// Create/Destroy
	NetObjectSendCreate			sendCreate;
	NetObjectReceiveCreate		receiveCreate;
	NetObjectSendDestroy		sendDestroy;
	NetObjectReceiveDestroy		receiveDestroy;

	// Snapshots
	size_t						snapshotSize;
	NetObjectMakeSnapshot		makeSnapshot;
	NetObjectSendSnapshot		sendSnapshot;
	NetObjectReceiveSnapshot	receiveSnapshot;
	NetObjectApplySnapshot		applySnapshot;
};