/************************************************************************/
/* File: RenderScene.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class to represent an object to be rendered (mesh and material)
/************************************************************************/
#pragma once
#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Math/Matrix44.hpp"

class Material;
class Transform;
class MeshBuilder;
class MaterialInstance;
class MeshGroup;

struct MaterialMeshSet
{
	Mesh* m_mesh = nullptr;
	Material*			m_sharedMaterial = nullptr;
	MaterialInstance*	m_materialInstance = nullptr;

	unsigned int m_vaoHandle = 0;
};

class Renderable
{
public:
	//-----Public Methods-----

	Renderable();
	Renderable(const Vector3& position, Mesh* mesh, Material* sharedMaterial);
	Renderable(const Matrix44& modelMatrix, Mesh* mesh, Material* sharedMaterial);
	Renderable(const Matrix44& modelMatrix, MeshGroup* meshGroup, Material* defaultMaterial);

	~Renderable();

	// Mutators
	void SetMesh(Mesh* mesh, unsigned int drawIndex);
	void SetSharedMaterial(Material* material, unsigned int drawIndex);
	void SetInstanceMaterial(MaterialInstance* instanceMaterial, unsigned int drawIndex);
	void SetMaterialMeshSet(unsigned int index, const MaterialMeshSet& set);

	void SetModelMatrix(const Matrix44& model, unsigned int instanceIndex);
	void AddModelMatrix(const Matrix44& model);
	void RemoveModelMatrix(unsigned int instanceIndex);

	// Accessors
	Mesh*			GetMesh(unsigned int drawIndex) const;
	Material*		GetSharedMaterial(unsigned int drawIndex) const;
	Material*		GetMaterialInstance(unsigned int drawIndex);
	Matrix44		GetModelMatrix(unsigned int instanceIndex) const;

	Material*		GetMaterialForRender(unsigned int drawIndex) const;

	unsigned int	GetVAOHandleForDraw(unsigned int drawIndex) const;

	// Producers
	Vector3 GetPosition(unsigned int instanceIndex) const;

	int		GetDrawCountPerInstance() const;
	int		GetInstanceCount() const;

	void ClearInstances();
	void ClearDraws();
	void ClearAll();


private:
	//-----Private Methods-----

	void BindMeshToMaterial(unsigned int drawIndex);


private:
	//-----Private Data-----

	std::vector<Matrix44> m_instanceModels;
	std::vector<MaterialMeshSet> m_matMeshSets;

};
