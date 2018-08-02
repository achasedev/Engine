/************************************************************************/
/* File: Renderable.hpp
/* Author: Andrew Chase
/* Date: May 2nd, 2018
/* Description: Class to represent an object to be rendered (mesh and material)
/************************************************************************/
#pragma once
#include "Engine/Math/Matrix44.hpp"
#include "Engine/Rendering/Meshes/Mesh.hpp"

class Material;
class MeshBuilder;
class MaterialInstance;

struct RenderableDraw_t
{
	Matrix44			drawMatrix;
	Mesh*				mesh = nullptr;
	Material*			sharedMaterial = nullptr;
	MaterialInstance*	materialInstance = nullptr;

	unsigned int vaoHandle = 0;
};

class Renderable
{
public:
	//-----Public Methods-----

	Renderable();
	~Renderable();

	// Mutators
	void AddDraw(const RenderableDraw_t& draw);
	void SetDraw(unsigned int index, RenderableDraw_t draw);

	void SetInstanceMatrix(unsigned int instanceIndex, const Matrix44& model);
	void AddInstanceMatrix(const Matrix44& model);
	void RemoveInstanceMatrix(unsigned int instanceIndex);

	void SetMesh(unsigned int index, Mesh* mesh);
	void SetModelMatrix(unsigned int index, const Matrix44& model);
	void SetSharedMaterial(unsigned int index, Material* sharedMaterial);
	void SetMaterialInstance(unsigned int index, MaterialInstance* materialInstance);


	// Accessors
	RenderableDraw_t	GetDraw(unsigned int drawIndex) const;

	Mesh*				GetMesh(unsigned int drawIndex) const;
	Material*			GetSharedMaterial(unsigned int drawIndex) const;
	Material*			GetMaterialInstance(unsigned int drawIndex);
	Matrix44			GetInstanceMatrix(unsigned int instanceIndex) const;

	Material*			GetMaterialForRender(unsigned int drawIndex) const;

	unsigned int		GetVAOHandleForDraw(unsigned int drawIndex) const;

	// Producers
	Vector3 GetInstancePosition(unsigned int instanceIndex) const;

	int		GetDrawCountPerInstance() const;
	int		GetInstanceCount() const;

	void	ClearInstances();
	void	ClearDraws();
	void	ClearAll();


private:
	//-----Private Methods-----

	void BindMeshToMaterial(unsigned int drawIndex);


private:
	//-----Private Data-----

	std::vector<Matrix44>			m_instanceModels;
	std::vector<RenderableDraw_t>	m_draws;

};
