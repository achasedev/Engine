/************************************************************************/
/* File: NamedProperties.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2019
/* Description: Classes for a generic, strongly-typed container of things
/************************************************************************/
#pragma once
#include <string>
#include <map>
#include "Engine/Core/Utility/StringUtils.hpp"

class BaseProperty
{
public:
	//-----Public Methods-----

	virtual std::string GetValueAsString() const = 0;
	virtual void const* GetTypeID() const = 0;

};


template <typename T>
class TypedProperty : public BaseProperty
{
public:
	//-----Public Methods-----

	//-----------------------------------------------------------------------------------------------
	virtual std::string GetValueAsString() const override
	{
		return ToString(m_value);
	}

	//-----------------------------------------------------------------------------------------------
	void SetValue(const T& newValue)
	{
		m_value = newValue;
	}

	//-----------------------------------------------------------------------------------------------
	T GetValue()
	{
		return m_value;
	}

	//-----------------------------------------------------------------------------------------------
	virtual void const* GetTypeID() const override
	{
		return &s_typeID;
	}

	//-----------------------------------------------------------------------------------------------
	static void const* GetTypeIDStatic()
	{
		return &s_typeID;
	}

private:
	//-----Private Data-----

	T m_value;

	static constexpr int s_typeID = 0; // To avoid using RTTI, comparing types

};


class NamedProperties
{
public:
	//-----Public Methods-----

	//-----------------------------------------------------------------------------------------------
	template <typename T>
	void Set(const std::string& name, const T& value)
	{
		bool alreadyExists = m_properties.find(name) != m_properties.end();
		
		if (alreadyExists) // Avoid type mismatching
		{
			BaseProperty* bp = m_properties[name];
			m_properties.erase(name);

			delete bp;
		}

		TypedProperty<T>* tp = new TypedProperty<T>();
		tp->SetValue(value);
		m_properties[name] = tp;
	}

	//-----------------------------------------------------------------------------------------------
	template <typename T>
	T Get(const std::string& name, const T& defaultValue)
	{
		bool propertyExists = m_properties.find(name) != m_properties.end();

		if (propertyExists)
		{
			BaseProperty* bp = m_properties[name];

			// Check types
			if (bp->GetTypeID() == TypedProperty<T>::GetTypeIDStatic())
			{
				TypedProperty<T>* tp = static_cast<TypedProperty<T>*>(bp);
				return tp->GetValue();
			}
		}

		// Property doesn't exist or is mismatched type, return the default
		return defaultValue;
	}

	//---String/Const Char* Helpers-----
	void		Set(const std::string& name, const char* value);
	std::string Get(const std::string& name, const char* defaultValue);

	std::string ToString() const;


private:
	//-----Private Data-----

	std::map<std::string, BaseProperty*> m_properties;

};
