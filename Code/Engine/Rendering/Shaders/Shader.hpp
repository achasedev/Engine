/************************************************************************/
/* File: Shader.hpp
/* Author: Andrew Chase
/* Date: April 8th, 2018
/* Description: Class to represent a shader program with render state
/************************************************************************/
#pragma once
#include <map>
#include <string>
#include "Engine/Rendering/OpenGL/glTypes.hpp"
#include "Engine/Core/Utility/XmlUtilities.hpp"

class ShaderProgram;

enum SortingQueue
{
	SORTING_QUEUE_OPAQUE,
	SORTING_QUEUE_ALPHA,
	NUM_SORTING_QUEUES

};

// Struct to represent non-programmable state
struct RenderState
{
	// Default constructor
	RenderState() {}

	// Full constructor, for use with ShaderSource
	RenderState(CullMode cull, FillMode fill, WindOrder wind, DepthTest depth, bool shouldWriteDepth, BlendOp colorOp, BlendFactor colorSrc, BlendFactor colorDst, BlendOp alphaOp, BlendFactor alphaSrc, BlendFactor alphaDst)
		: m_cullMode(cull), m_fillMode(fill), m_windOrder(wind)
		, m_depthTest(depth), m_shouldWriteDepth(shouldWriteDepth)
		, m_colorBlendOp(colorOp), m_colorSrcFactor(colorSrc), m_colorDstFactor(colorDst)
		, m_alphaBlendOp(alphaOp), m_alphaSrcFactor(alphaSrc), m_alphaDstFactor(alphaDst)
	{}

	// Rasterization State Control
	CullMode	m_cullMode = CULL_MODE_BACK;
	FillMode	m_fillMode = FILL_MODE_SOLID;
	WindOrder	m_windOrder = WIND_COUNTER_CLOCKWISE;

	// Depth State Control
	DepthTest	m_depthTest = DEPTH_TEST_LESS;
	bool		m_shouldWriteDepth = true;

	// Blend State Control
	BlendOp		m_colorBlendOp = BLEND_OP_ADD;
	BlendFactor m_colorSrcFactor = BLEND_FACTOR_ONE;
	BlendFactor m_colorDstFactor = BLEND_FACTOR_ZERO;

	BlendOp		m_alphaBlendOp = BLEND_OP_ADD;
	BlendFactor m_alphaSrcFactor = BLEND_FACTOR_ONE;
	BlendFactor m_alphaDstFactor = BLEND_FACTOR_ONE;
};


class Shader
{
public:
	//-----Public Methods-----

	Shader(ShaderProgram* program);
	Shader(const RenderState& renderState, ShaderProgram* program);
	Shader(const std::string& xmlFileName);
	~Shader();

	Shader* Clone();

	static Shader* BuildShader(const std::string& programName, const char* vsSource, const char* fsSource, 
		const RenderState& state, unsigned int sortingLayer, SortingQueue sortingQueue);

	// Mutators
	void SetProgram(ShaderProgram* program);

	void SetCullMode(CullMode mode);
	void SetFillMode(FillMode mode);
	void SetWindOrder(WindOrder order);

	void EnableDepth(DepthTest test, bool shouldWrite);
	void DisableDepth();

	void EnableColorBlending(BlendOp op, BlendFactor srcFactor, BlendFactor dstFactor);
	void EnableAlphaBlending(BlendOp op, BlendFactor srcFactor, BlendFactor dstFactor);

	void DisableColorBlending();
	void DisableAlphaBlending();

	// Accessors
	ShaderProgram*			GetProgram() const;
	const RenderState&		GetRenderState() const;

	unsigned int	GetLayer() const;
	SortingQueue	GetQueue() const;

private:
	//-----Private Methods-----

	// For XML Parsing
	void ParseProgram(const XMLElement& shaderElement);
	void ParseCullMode(const XMLElement& shaderElement);
	void ParseFillMode(const XMLElement& shaderElement);
	void ParseWindOrder(const XMLElement& shaderElement);
	void ParseDepthMode(const XMLElement& shaderElement);
	void ParseBlendMode(const XMLElement& shaderElement);


private:
	//-----Private Data-----

	ShaderProgram*	m_shaderProgram;
	RenderState		m_renderState;

	// For the forward rendering path, is ignored elsewhere
	unsigned int m_layer = 0;
	SortingQueue m_queue = SORTING_QUEUE_OPAQUE;

};
