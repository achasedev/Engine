/************************************************************************/
/* File: AssetCollection.hpp
/* Author: Andrew Chase
/* Date: April 11th, 2018
/* Description: Class to represent a collection of a single asset type
/************************************************************************/
#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"

template <typename RESOURCETYPE>
class AssetCollection
{
	// Only the database can access any members of this class
	friend class AssetDB;

private:
	//-----Private Methods-----

	static RESOURCETYPE*	GetAsset(const std::string& name);
	static bool				AddAsset(const std::string& name, RESOURCETYPE* resource);


private:
	//-----Private Data-----

	static std::map<std::string, RESOURCETYPE*> s_collection;

};


//-----------------------------------------------------------------------------------------------
// Returns the resource given by the name, returns nullptr if not found
//
template <typename RESOURCETYPE>
std::map<std::string, RESOURCETYPE*> AssetCollection<RESOURCETYPE>::s_collection;

template <typename RESOURCETYPE>
RESOURCETYPE* AssetCollection<RESOURCETYPE>::GetAsset(const std::string& name)
{
	bool resourceExists = (s_collection.find(name) != s_collection.end());
	
	if (resourceExists)
	{
		return s_collection[name];
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Adds the resource to the collection, checking for duplicates
//
template <typename RESOURCETYPE>
bool AssetCollection<RESOURCETYPE>::AddAsset(const std::string& name, RESOURCETYPE* resource)
{
	bool resourceAlreadyExists = s_collection.find(name) != s_collection.end();

	if (!resourceAlreadyExists)
	{
		s_collection[name] = resource;
	}

	return !resourceAlreadyExists;
}
