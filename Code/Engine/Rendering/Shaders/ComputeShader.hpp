#pragma once

class ComputeShader
{
public:
	//-----Public Methods----

	ComputeShader();
	~ComputeShader();

	bool Initialize(const char* filename);

	void Execute(int totalXItems, int totalYItems, int totalZItems);

private:
	//-----Private Data-----

	unsigned int m_programHandle = 0;

};
