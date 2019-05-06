/************************************************************************/
/* File: EventSubscription.cpp
/* Author: Andrew Chase
/* Date: May 5th 2019
/* Description: Implementation of the EventSubscription subclasses used by
/*				the EventSystem
/************************************************************************/
#include "Engine/Core/EventSystem/EventSubscription.hpp"


//-----------------------------------------------------------------------------------------------
// Constructor for an event that calls a standalone C function or a class static function
//
EventFunctionSubscription::EventFunctionSubscription(EventFunctionCallback callback)
	: m_functionCallback(callback)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor for an event that calls a standalone C function or a class static function
//
EventFunctionSubscription::~EventFunctionSubscription()
{
	m_functionCallback = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Calls the function callback for this subscription
//
bool EventFunctionSubscription::Execute(NamedProperties& args)
{
	return m_functionCallback(args);
}

