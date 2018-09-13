#pragma once

class ComputeShader
{
public:
	//-----Public Methods----

	ComputeShader();
	~ComputeShader();

	bool Initialize(const char* filename);

	void Execute(int numGroupsX, int numGroupsY, int numGroupsZ);
	void Execute(int numGroupsX, int numGroupsY, int numGroupsZ, 
		int groupSizeX, int groupSizeY, int groupSizeZ);

private:
	//-----Private Data-----

	unsigned int m_programHandle = 0;

};
