/************************************************************************/
/* File: NetObjectConnectionView.hpp
/* Author: Andrew Chase
/* Date: December 3rd 2018
/* Description: Class to represent a single connection's view of a set of
/*				NetObjects as seen from the host
/************************************************************************/
#pragma once
#include <vector>

class NetObject;
class NetObjectView;

class NetObjectConnectionView
{
public:
	//-----Public Methods-----
	
	~NetObjectConnectionView();

	void AddNetObjectView(NetObjectView* objectView);
	void AddNetObjectView(NetObject* netObject);
	void RemoveNetObjectView(NetObject* netObject);

	int GetViewCount() const;
	NetObjectView* GetNextObjectViewToSendUpdateFor() const;

	
private:
	//-----Private Data-----
	
	std::vector<NetObjectView*> m_objectViews;

};
