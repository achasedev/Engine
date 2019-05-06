/************************************************************************/
/* File: EventSystem.cpp
/* Author: Andrew Chase
/* Date: May 5th 2019
/* Description: Implementation of the EventSystem singleton class
/************************************************************************/
#include "Engine/Core/EventSystem/EventSystem.hpp"
#include "Engine/DataStructures/NamedProperties.hpp"
#include "Engine/Core/Utility/ErrorWarningAssert.hpp"

// Singleton instance
EventSystem* EventSystem::s_instance = nullptr;


//-----------------------------------------------------------------------------------------------
// Constructor
//
EventSystem::EventSystem()
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
EventSystem::~EventSystem()
{
}


//-----------------------------------------------------------------------------------------------
// Creates the singleton instance
//
void EventSystem::Initialize()
{
	ASSERT_OR_DIE(s_instance == nullptr, "EventSystem::Initialize() called twice!");
	s_instance = new EventSystem();
}


//-----------------------------------------------------------------------------------------------
// Deletes the singleton instance
//
void EventSystem::Shutdown()
{
	if (s_instance != nullptr)
	{
		delete s_instance;
		s_instance = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns the singleton instance
//
EventSystem* EventSystem::GetInstance()
{
	return s_instance;
}


//-----------------------------------------------------------------------------------------------
// Adds a subscription for the given function callback
//
void EventSystem::SubscribeEventCallbackFunction(const char* eventNameToSubTo, EventFunctionCallback callback)
{
	EventFunctionSubscription* subscription = new EventFunctionSubscription(callback);

	// This creates the entry if there isn't one already
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventNameToSubTo];
	subsToEvent.push_back(subscription);
}


//-----------------------------------------------------------------------------------------------
// Removes the function subscription from the event listed by eventNameToUnsubFrom
//
void EventSystem::UnsubscribeEventCallbackFunction(const char* eventNameToUnsubFrom, EventFunctionCallback callback)
{
	std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventNameToUnsubFrom];

	int numSubs = (int)subsToEvent.size();
	for (int subIndex = 0; subIndex < numSubs; ++subIndex)
	{
		EventFunctionSubscription* currSub = dynamic_cast<EventFunctionSubscription*>(subsToEvent[subIndex]);

		if (currSub != nullptr) // currSub is a standalone function subscription
		{
			if (currSub->m_functionCallback == callback) // currSub is the one for the given callback
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
	LogTaggedPrintf("EVENT", "Tried to unsubscribe a function subscription from event named \"%s\" but couldn't find it");
}


//-----------------------------------------------------------------------------------------------
// Calls all callbacks that are subscribed to the event given by eventName
//
void EventSystem::FireEvent(const char* eventName, NamedProperties& args)
{
	bool subsToEventExist = (m_subscriptions.find(eventName) != m_subscriptions.end());

	if (subsToEventExist)
	{
		std::vector<EventSubscription*>& subsToEvent = m_subscriptions[eventName];

		int numSubs = (int)subsToEvent.size();

		for (int subIndex = 0; subIndex < numSubs; ++subIndex)
		{
			bool subConsumedEvent = subsToEvent[subIndex]->Execute(args);

			if (subConsumedEvent)
			{
				break;
			}
		}
	}
}


//---C FUNCTION----------------------------------------------------------------------------------
// Shortcut function for firing an event by name with no parameters
//
void FireEvent(const char* name)
{
	NamedProperties args;
	FireEvent(name, args);
}


//---C FUNCTION----------------------------------------------------------------------------------
// Shortcut function for firing an event by name on the singleton EventSystem instance
//
void FireEvent(const char* eventName, NamedProperties& args)
{
	EventSystem* eventSystem = EventSystem::GetInstance();
	eventSystem->FireEvent(eventName, args);
}
