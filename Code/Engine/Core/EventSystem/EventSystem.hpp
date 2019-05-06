/************************************************************************/
/* File: EventSystem.hpp
/* Author: Andrew Chase
/* Date: May 5th 2019
/* Description: Class for the singleton event system of the engine
/************************************************************************/
#pragma once
#include "Engine/Core/LogSystem.hpp"
#include "Engine/Core/EventSystem/EventSubscription.hpp"
#include <map>
#include <string>
#include <vector>

class NamedProperties;
class EventSystem
{
public:
	//-----Public Methods-----

	static void			Initialize();
	static void			Shutdown();
	static EventSystem* GetInstance();

	void SubscribeEventCallbackFunction(const char* eventNameToSubTo, EventFunctionCallback callback);
	void UnsubscribeEventCallbackFunction(const char* eventNameToUnsubFrom, EventFunctionCallback callback);

	template <typename T, typename T_Method>
	void SubscribeEventCallbackObjectMethod(const char* eventNameToSubTo, T_Method callback, T& object);
	template <typename T, typename T_Method>
	void UnsubscribeEventCallbackObjectMethod(const char* eventNameToUnsubFrom, T_Method callback, T& object);

	void FireEvent(const char* eventName, NamedProperties& args);


private:
	//-----Private Methods-----

	// Singleton, use Initialize() instead
	EventSystem();
	~EventSystem();
	EventSystem(const EventSystem& copy) = delete;


private:
	//-----Private Data-----

	std::map<std::string, std::vector<EventSubscription*>> m_subscriptions;

	static EventSystem* s_instance;

};

//////////////////////////////////////////////////////////////////////////
// Template Implementations
//////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------------
// Creates and adds an object method subscription for the given object and callback
//
template <typename T, typename T_Method>
void EventSystem::SubscribeEventCallbackObjectMethod(const char* eventNameToSubTo, T_Method callback, T& object)
{
	EventObjectMethodSubscription<T>* subscription = new EventObjectMethodSubscription<T>(callback, object);

	// This creates the entry if there isn't one already
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventNameToSubTo];
	subsToEvent.push_back(subscription);
}


//-----------------------------------------------------------------------------------------------
// Removes the given subscription from the list of subscribers for the given event
//
template <typename T, typename T_Method>
void EventSystem::UnsubscribeEventCallbackObjectMethod(const char* eventNameToUnsubFrom, T_Method callback, T& object)
{
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventNameToUnsubFrom];

	int numSubs = (int)subsToEvent.size();
	for (int subIndex = 0; subIndex < numSubs; ++subIndex)
	{
		EventObjectMethodSubscription<T>* currSub = dynamic_cast<EventObjectMethodSubscription<T>*>(subsToEvent[subIndex]);

		if (currSub != nullptr) // currSub is an object method subscription
		{
			if (currSub->m_methodCallback == callback && &currSub->m_object == &object) // currSub is the one for the given object and callback
			{
				subsToEvent.erase(subsToEvent.begin() + subIndex);

				if (subsToEvent.size() == 0)
				{
					m_subscriptions.erase(eventNameToUnsubFrom);
				}

				delete currSub;
				return;
			}
		}
	}

	// This is only reached if we don't find an event for this object and method callback
	LogTaggedPrintf("EVENT", "Tried to unsubscribe an object method subscription from event named \"%s\" but couldn't find it");
}


//////////////////////////////////////////////////////////////////////////
// C Function Shortcuts
//////////////////////////////////////////////////////////////////////////
void FireEvent(const char* name);
void FireEvent(const char* eventName, NamedProperties& args);
