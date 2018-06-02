/************************************************************************/
/* File: RenderScene.cpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Implementation of the renderable class
/************************************************************************/
#include "Engine/Math/Transform.hpp"
#include "Engine/Rendering/Core/Renderer.hpp"
#include "Engine/Rendering/Meshes/MeshGroup.hpp"
#include "Engine/Rendering/Core/Renderable.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Core/RenderScene.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor from members
//
Renderable::Renderable(const Matrix44& model, Mesh* mesh, Material* sharedMaterial)
{
	ClearAll();

	m_instanceModels.push_back(model);

	MaterialMeshSet set;
	set.m_sharedMaterial = sharedMaterial;
	set.m_mesh = mesh;

	m_matMeshSets.push_back(set);

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
	// Always ensure one is in place
	m_instanceModels.push_back(Matrix44::IDENTITY);

	MaterialMeshSet set;
	m_matMeshSets.push_back(set);
}


//-----------------------------------------------------------------------------------------------
// Constructor - from a mesh group. Assigns the default material to all meshes
//
Renderable::Renderable(const Matrix44& model, MeshGroup* meshGroup, Material* defaultMaterial)
{
	ClearAll();

	m_instanceModels.push_back(model);

	int numMeshes = meshGroup->GetMeshCount();
	for (int meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
	{
		MaterialMeshSet set;
		set.m_sharedMaterial = defaultMaterial;
		set.m_mesh = meshGroup->GetMesh(meshIndex);

		m_matMeshSets.push_back(set);
		BindMeshToMaterial(meshIndex);
	}
}


//-----------------------------------------------------------------------------------------------
// Constructor - from a position
//
Renderable::Renderable(const Vector3& position, Mesh* mesh, Material* sharedMaterial)
{
	ClearAll();

	Matrix44 model = Matrix44::MakeModelMatrix(position, Vector3::ZERO, Vector3::ONES);
	m_instanceModels.push_back(model);

	MaterialMeshSet set;
	set.m_sharedMaterial = sharedMaterial;
	set.m_mesh = mesh;

	m_matMeshSets.push_back(set);
	BindMeshToMaterial(0);
}


//-----------------------------------------------------------------------------------------------
// Destructor - used to delete an instance material if one was used
//
Renderable::~Renderable()
{
	ClearAll();
}


//-----------------------------------------------------------------------------------------------
// Sets the mesh on the renderable to the one specified
//
void Renderable::SetMesh(Mesh* mesh, unsigned int drawIndex)
{
	m_matMeshSets[drawIndex].m_mesh = mesh;
	BindMeshToMaterial(drawIndex);
}


//-----------------------------------------------------------------------------------------------
// Sets the shared material on the renderable to the one specified
// Also deletes the instance material if one was created from the previous shared material
//
void Renderable::SetSharedMaterial(Material* material, unsigned int drawIndex)
{
	// If we have a material instance, delete it
	// We'll render with the new shared material
	if (m_matMeshSets[drawIndex].m_materialInstance != nullptr)
	{
		delete m_matMeshSets[drawIndex].m_materialInstance;
		m_matMeshSets[drawIndex].m_materialInstance = nullptr;
	}

	m_matMeshSets[drawIndex].m_sharedMaterial = material;
	BindMeshToMaterial(drawIndex);
}


//-----------------------------------------------------------------------------------------------
// Sets the material instance to the one specified, deleting the old one if there was one
//
void Renderable::SetInstanceMaterial(MaterialInstance* instanceMaterial, unsigned int drawIndex)
{
	if (m_matMeshSets[drawIndex].m_materialInstance != nullptr)
	{
		delete m_matMeshSets[drawIndex].m_materialInstance;
	}

	m_matMeshSets[drawIndex].m_materialInstance = instanceMaterial;
	BindMeshToMaterial(drawIndex);
}


//-----------------------------------------------------------------------------------------------
// Sets the material/mesh set at the given index to that specified
//
void Renderable::SetMaterialMeshSet(unsigned int index, const MaterialMeshSet& set)
{
	m_matMeshSets[index] = set;
	BindMeshToMaterial(index);
}


//-----------------------------------------------------------------------------------------------
// Sets the model matrix at the given index to the one specified
// For instanced rendering
//
void Renderable::SetModelMatrix(const Matrix44& model, unsigned int instanceIndex)
{
	m_instanceModels[instanceIndex] = model;
}

//-----------------------------------------------------------------------------------------------
// Adds the given matrix to the list of instanced model matrices
// Used for instance drawing
//
void Renderable::AddModelMatrix(const Matrix44& model)
{
	m_instanceModels.push_back(model);
}


//-----------------------------------------------------------------------------------------------
// Removes the matrix at the given index
//
void Renderable::RemoveModelMatrix(unsigned int instanceIndex)
{
	m_instanceModels.erase(m_instanceModels.begin() + instanceIndex);
}


//-----------------------------------------------------------------------------------------------
// Returns the mesh of the renderable
//
Mesh* Renderable::GetMesh(unsigned int drawIndex) const
{
	return m_matMeshSets[drawIndex].m_mesh;
}


//-----------------------------------------------------------------------------------------------
// Returns the shared material of the renderable
//
Material* Renderable::GetSharedMaterial(unsigned int drawIndex) const
{
	return m_matMeshSets[drawIndex].m_sharedMaterial;
}


//-----------------------------------------------------------------------------------------------
// Returns the material instance of the renderable, creating one from the shared material if it
// one wasn't created yet
//
Material* Renderable::GetMaterialInstance(unsigned int drawIndex)
{
	// Make an instance now if one doesn't exist
	if (m_matMeshSets[drawIndex].m_materialInstance == nullptr)
	{
		m_matMeshSets[drawIndex].m_materialInstance = new MaterialInstance(m_matMeshSets[drawIndex].m_sharedMaterial);
	}

	return m_matMeshSets[drawIndex].m_materialInstance;
}


//-----------------------------------------------------------------------------------------------
// Returns the model matrix of the renderable instance at the index
//
Matrix44 Renderable::GetModelMatrix(unsigned int instanceIndex) const
{
	return m_instanceModels[instanceIndex];
}


//-----------------------------------------------------------------------------------------------
// Returns the instance material if one was created, otherwise returns the shared material
//
Material* Renderable::GetMaterialForRender(unsigned int drawIndex) const
{
	if (m_matMeshSets[drawIndex].m_materialInstance != nullptr)
	{
		return m_matMeshSets[drawIndex].m_materialInstance;
	}
	
	return m_matMeshSets[drawIndex].m_sharedMaterial;
}


//-----------------------------------------------------------------------------------------------
// Returns the Vertex Array Object handle for the given draw of this renderable
//
unsigned int Renderable::GetVAOHandleForDraw(unsigned int drawIndex) const
{
	return m_matMeshSets[drawIndex].m_vaoHandle;
}


//-----------------------------------------------------------------------------------------------
// Returns the position of the renderable if it has a transform, or (0,0,0) otherwise
//
Vector3 Renderable::GetPosition(unsigned int instanceIndex) const
{
	return m_instanceModels[instanceIndex].GetTVector().xyz();
}


//-----------------------------------------------------------------------------------------------
// Returns the number of meshes in this renderable
//
int Renderable::GetDrawCountPerInstance() const
{
	return (int) m_matMeshSets.size();
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
	int numSets = (int) m_matMeshSets.size();

	Renderer* renderer = Renderer::GetInstance();

	for (int setIndex = 0; setIndex < numSets; ++setIndex)
	{
		if (m_matMeshSets[setIndex].m_materialInstance != nullptr)
		{
			delete m_matMeshSets[setIndex].m_materialInstance;

			// Also check to free the VAO
			renderer->DeleteVAO(m_matMeshSets[setIndex].m_vaoHandle);
		}
	}

	m_matMeshSets.clear();
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
	Mesh* mesh = m_matMeshSets[drawIndex].m_mesh;
	Material* material = GetMaterialForRender(drawIndex);

	// Don't bind them to the VAO if one is missing
	if (mesh == nullptr || material == nullptr)
	{
		return;
	}

	Renderer* renderer = Renderer::GetInstance();
	renderer->UpdateVAO(m_matMeshSets[drawIndex].m_vaoHandle, mesh, material);
}
