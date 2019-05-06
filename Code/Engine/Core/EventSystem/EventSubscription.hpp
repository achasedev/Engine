/************************************************************************/
/* File: EventSubscription.hpp
/* Author: Andrew Chase
/* Date: May 5th 2019
/* Description: Classes for representing subscriptions within the Event
/*				System
/************************************************************************/
#pragma once

class NamedProperties;
typedef bool(*EventFunctionCallback)(NamedProperties& args);

class EventSubscription
{
public:
	//-----Public Methods-----
	EventSubscription() {}
	virtual ~EventSubscription() {}
	virtual bool Execute(NamedProperties& args) = 0;
};


class EventFunctionSubscription : public EventSubscription
{
	friend class EventSystem;

public:
	//-----Public Methods-----

	EventFunctionSubscription(EventFunctionCallback callback);
	virtual ~EventFunctionSubscription();
	virtual bool Execute(NamedProperties& args) override;


private:
	//-----Private Data-----

	EventFunctionCallback m_functionCallback = nullptr;

};


template <typename T>
class EventObjectMethodSubscription : public EventSubscription
{
public:

	friend class EventSystem;

	// For saving off the function callback for each templated class version
	typedef bool(T::*EventObjectMethodCallback)(NamedProperties& args);


public:
	//-----Public Methods-----

	EventObjectMethodSubscription(EventObjectMethodCallback callback, T& object);
	virtual ~EventObjectMethodSubscription();
	virtual bool Execute(NamedProperties& args) override;


private:
	//-----Private Data-----

	T& m_object;
	EventObjectMethodCallback m_methodCallback = nullptr;

};


//////////////////////////////////////////////////////////////////////////
// Template Implementations
//////////////////////////////////////////////////////////////////////////


//-----------------------------------------------------------------------------------------------
// Constructor
//
template <typename T>
EventObjectMethodSubscription<T>::EventObjectMethodSubscription(EventObjectMethodCallback callback, T& object)
	: m_methodCallback(callback)
	, m_object(object)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor
//
template <typename T>
EventObjectMethodSubscription<T>::~EventObjectMethodSubscription()
{
	m_methodCallback = nullptr;
}


//-----------------------------------------------------------------------------------------------
// Calls the method callback on the given object for this subscription
//
template <typename T>
bool EventObjectMethodSubscription<T>::Execute(NamedProperties& args)
{
	return (m_object.*m_methodCallback)(args);
}
