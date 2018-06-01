/************************************************************************/
/* File: DrawCall.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the DrawCall class
/************************************************************************/
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/DrawCall.hpp"
#include "Engine/Renderer/Renderable.hpp"


//-----------------------------------------------------------------------------------------------
// Returns the mesh of the draw call
//
Mesh* DrawCall::GetMesh() const
{
	return m_mesh;
}


//-----------------------------------------------------------------------------------------------
// Returns the model matrix of the draw call
//
Matrix44 DrawCall::GetModelMatrix(unsigned int index) const
{
	return m_modelMatrices[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the collection of matrices used for an instanced draw call
//
const Matrix44* DrawCall::GetModelMatrixBuffer() const
{
	return m_modelMatrices.data();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of model matrices used by this draw call, count > 1 indicates it's instanced
// rendering
//
int DrawCall::GetModelMatrixCount() const
{
	return (int) m_modelMatrices.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the Vertex Array Object handle for this draw call
//
unsigned int DrawCall::GetVAOHandle() const
{
	return m_vaoHandle;
}


//-----------------------------------------------------------------------------------------------
// Returns the material of the draw call
//
Material* DrawCall::GetMaterial() const
{
	return m_material;
}


//-----------------------------------------------------------------------------------------------
// Calculates the overall sort order for the draw call given its layer and queue order
//
int DrawCall::GetSortOrder() const
{
	return m_layer * NUM_SORTING_QUEUES + m_renderQueue;
}


//-----------------------------------------------------------------------------------------------
// Returns the number of lights used by this draw call
//
int DrawCall::GetNumLights() const
{
	return m_numLightsInUse;
}


//-----------------------------------------------------------------------------------------------
// Returns the light at the given index
//
Light* DrawCall::GetLight(unsigned int index) const
{
	return m_lights[index];
}


//-----------------------------------------------------------------------------------------------
// Returns the ambient light used for this draw
//
Rgba DrawCall::GetAmbience() const
{
	return m_ambience;
}


//-----------------------------------------------------------------------------------------------
// Sets all members to be that from the renderable given
// Returns false if there are no model instances for the renderable, meaning no need to draw
//
bool DrawCall::SetDataFromRenderable(Renderable* renderable, int dcIndex)
{
	m_mesh = renderable->GetMesh(dcIndex);
	m_material = renderable->GetMaterialForRender(dcIndex);

	// Set all the matrix data
	m_modelMatrices.clear();

	int numMatrices = renderable->GetInstanceCount();

	if (numMatrices == 0)
	{
		return false;
	}

	for (int matrixIndex = 0; matrixIndex < numMatrices; ++matrixIndex)
	{
		m_modelMatrices.push_back(renderable->GetModelMatrix(matrixIndex));
	}

	const Shader* shader = m_material->GetShader();
	m_layer = shader->GetLayer();
	m_renderQueue = shader->GetQueue();

	// Set the VAO handle
	m_vaoHandle = renderable->GetVAOHandleForDraw(dcIndex);

	return true;
}


//-----------------------------------------------------------------------------------------------
// Sets the ambient light value for this draw to the value specified
//
void DrawCall::SetAmbience(const Rgba& ambience)
{
	m_ambience = ambience;
}


//-----------------------------------------------------------------------------------------------
// Sets the light at the given index of the draw call to the one specified
//
void DrawCall::SetLight(unsigned int index, Light* light)
{
	m_lights[index] = light;
}


//-----------------------------------------------------------------------------------------------
// Sets the total number of lights this draw call is using
// Draw calls support up to 8 in its array of lights, but this value is the actual number used
//
void DrawCall::SetNumLightsInUse(unsigned int lightsInUse)
{
	m_numLightsInUse = lightsInUse;
}
