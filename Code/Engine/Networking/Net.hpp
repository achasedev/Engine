/************************************************************************/
/* File: Net.hpp
/* Author: Andrew Chase
/* Date: August 20th, 2018
/* Description: Static class for handling networking operations
/************************************************************************/
#pragma once
#pragma comment(lib, "ws2_32.lib") // winsock libraries

class Net
{
public:
	//-----Public Methods-----

	static bool Initialize();
	static void Shutdown();
};