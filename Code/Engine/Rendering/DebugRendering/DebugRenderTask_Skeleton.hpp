/************************************************************************/
/* File: DebugRenderTask_Skeleton.hpp
/* Author: Andrew Chase
/* Date: June 15th, 2018
/* Description: Class to represent an animation skeleton debug rendering
/************************************************************************/
#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Rendering/DebugRendering/DebugRenderTask.hpp"

class Skeleton;

class DebugRenderTask_Skeleton : public DebugRenderTask
{
public:
	//-----Public Methods-----

	DebugRenderTask_Skeleton(const Skeleton* skeleton, const Matrix44& transform, const DebugRenderOptions& options);

	virtual void Render() const override;


private:
	//-----Private Methods-----

	void AssembleMesh(const Skeleton* skeleton);

};