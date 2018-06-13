/************************************************************************/
/* File: RenderScene.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the renderable class
/************************************************************************/
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor from members
//
Renderable::Renderable(const Matrix44& model, Mesh* mesh, Material* sharedMaterial)
{
	m_instanceModels.push_back(model);

	RenderableDraw_t draw;
	draw.sharedMaterial = sharedMaterial;
	draw.mesh = mesh;

	m_draws.push_back(draw);

	if (mesh != nullptr && sharedMaterial != nullptr)
	{
		BindMeshToMaterial(0);
	}
}


//-----------------------------------------------------------------------------------------------
// Default constructor
//
Renderable::Renderable()
{	
}


//-----------------------------------------------------------------------------------------------
// Destructor - used to delete an instance material if one was used
//
Renderable::~Renderable()
{
	ClearAll();
}


//-----------------------------------------------------------------------------------------------
// Adds a draw to the list of draws for each instance
//
void Renderable::AddDraw(const RenderableDraw_t& draw)
{
	m_draws.push_back(draw);
	BindMeshToMaterial((int) m_draws.size() - 1);
}


//-----------------------------------------------------------------------------------------------
// Sets the model matrix at the given index to the one specified
// For instanced rendering
//
void Renderable::SetInstanceMatrix(unsigned int instanceIndex, const Matrix44& model)
{
	m_instanceModels[instanceIndex] = model;
}

//-----------------------------------------------------------------------------------------------
// Adds the given matrix to the list of instanced model matrices
// Used for instance drawing
//
void Renderable::AddInstanceMatrix(const Matrix44& model)
{
	m_instanceModels.push_back(model);
}


//-----------------------------------------------------------------------------------------------
// Removes the matrix at the given index
//
void Renderable::RemoveInstanceMatrix(unsigned int instanceIndex)
{
	m_instanceModels.erase(m_instanceModels.begin() + instanceIndex);
}


//-----------------------------------------------------------------------------------------------
// Sets the base skeleton (bind pose) for the renderable
//
void Renderable::SetSkeleton(const SkeletonBase* skeleton)
{
	m_skeleton = skeleton;
}


//-----------------------------------------------------------------------------------------------
// Returns the given draw object at the index
//
RenderableDraw_t Renderable::GetDraw(unsigned int drawIndex) const
{
	return m_draws[drawIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the mesh of the renderable
//
Mesh* Renderable::GetMesh(unsigned int drawIndex) const
{
	return m_draws[drawIndex].mesh;
}


//-----------------------------------------------------------------------------------------------
// Returns the shared material of the renderable
//
Material* Renderable::GetSharedMaterial(unsigned int drawIndex) const
{
	return m_draws[drawIndex].sharedMaterial;
}


//-----------------------------------------------------------------------------------------------
// Returns the material instance of the renderable, creating one from the shared material if it
// one wasn't created yet
//
Material* Renderable::GetMaterialInstance(unsigned int drawIndex)
{
	// Make an instance now if one doesn't exist
	if (m_draws[drawIndex].materialInstance == nullptr)
	{
		m_draws[drawIndex].materialInstance = new MaterialInstance(m_draws[drawIndex].sharedMaterial);
	}

	return m_draws[drawIndex].materialInstance;
}


//-----------------------------------------------------------------------------------------------
// Returns the model matrix of the renderable instance at the index
//
Matrix44 Renderable::GetInstanceMatrix(unsigned int instanceIndex) const
{
	return m_instanceModels[instanceIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the instance material if one was created, otherwise returns the shared material
//
Material* Renderable::GetMaterialForRender(unsigned int drawIndex) const
{
	if (m_draws[drawIndex].materialInstance != nullptr)
	{
		return m_draws[drawIndex].materialInstance;
	}
	
	return m_draws[drawIndex].sharedMaterial;
}


//-----------------------------------------------------------------------------------------------
// Returns the Vertex Array Object handle for the given draw of this renderable
//
unsigned int Renderable::GetVAOHandleForDraw(unsigned int drawIndex) const
{
	return m_draws[drawIndex].vaoHandle;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the renderable if it has a transform, or (0,0,0) otherwise
//
Vector3 Renderable::GetInstancePosition(unsigned int instanceIndex) const
{
	return m_instanceModels[instanceIndex].GetTVector().xyz();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of meshes in this renderable
//
int Renderable::GetDrawCountPerInstance() const
{
	return (int) m_draws.size();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of instances this renderable has in a single draw
//
int Renderable::GetInstanceCount() const
{
	return (int) m_instanceModels.size();
}


//-----------------------------------------------------------------------------------------------
// Clears the instance matrix data
//
void Renderable::ClearInstances()
{
	m_instanceModels.clear();
}


//-----------------------------------------------------------------------------------------------
// Clears the material/mesh information
//
void Renderable::ClearDraws()
{
	int numSets = (int) m_draws.size();

	Renderer* renderer = Renderer::GetInstance();

	for (int setIndex = 0; setIndex < numSets; ++setIndex)
	{
		if (m_draws[setIndex].materialInstance != nullptr)
		{
			delete m_draws[setIndex].materialInstance;

			// Also check to free the VAO
			renderer->DeleteVAO(m_draws[setIndex].vaoHandle);
		}
	}

	m_draws.clear();
}


//-----------------------------------------------------------------------------------------------
// Clears the mesh/material list and the instance model, deleting any memory instanced by this renderable
//
void Renderable::ClearAll()
{
	ClearInstances();
	ClearDraws();
}


//-----------------------------------------------------------------------------------------------
// Updates the VAO between the mesh and the material for the given draw index
//
void Renderable::BindMeshToMaterial(unsigned int drawIndex)
{
	Mesh* mesh = m_draws[drawIndex].mesh;
	Material* material = GetMaterialForRender(drawIndex);

	// Don't bind them to the VAO if one is missing
	if (mesh == nullptr || material == nullptr)
	{
		return;
	}

	Renderer* renderer = Renderer::GetInstance();
	renderer->UpdateVAO(m_draws[drawIndex].vaoHandle, mesh, material);
}
