/************************************************************************/
/* File: NetObjectType.hpp
/* Author: Andrew Chase
/* Date: December 2nd 2018
/* Description: Header file for a Network Object type definition
/************************************************************************/
#pragma once
#include <stdint.h>

class NetMessage;

typedef void(*NetObjectWriteCreate)(NetMessage& msg, void* object);
typedef void*(*NetObjectReadCreate)(NetMessage& msg);
typedef void(*NetObjectWriteDestroy)(NetMessage& msg, void* object);
typedef void(*NetObjectReadDestroy)(NetMessage* msg, void* object);

typedef void(*NetObjectMakeSnapshot)(void* snapshot, const void* object);
typedef void(*NetObjectWriteSnapshot)(NetMessage& msg, const void* snapshot);
typedef void(*NetObjectReadSnapshot)(NetMessage& msg, void* out_snapshot);
typedef void(*NetObjectApplySnapshot)(void* snapshot, void* object);

struct NetObjectType_t
{
	// ID
	uint8_t	id;

	// Create/Destroy
	NetObjectWriteCreate		writeCreate;
	NetObjectReadCreate			readCreate;
	NetObjectWriteDestroy		writeDestroy;
	NetObjectReadDestroy		readDestroy;

	// Snapshots
	size_t						snapshotSize;
	NetObjectMakeSnapshot		makeSnapshot;
	NetObjectWriteSnapshot		writeSnapshot;
	NetObjectReadSnapshot		readSnapshot;
	NetObjectApplySnapshot		applySnapshot;

};