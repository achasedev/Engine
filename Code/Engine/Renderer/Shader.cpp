/************************************************************************/
/* File: Shader.cpp
/* Author: Andrew Chase
/* Date: April 8th, 2018
/* Description: Implementation of the Shader class
/************************************************************************/
#include "Engine/Core/AssetDB.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/ShaderProgram.hpp"
#include "Engine/Renderer/ShaderSource.hpp"

//-----------------------------------------------------------------------------------------------
// Constructor - from literal data, render state defaults to values in .hpp
//
Shader::Shader(const ShaderProgram* program)
	: m_shaderProgram(program)
{
}


//-----------------------------------------------------------------------------------------------
// Constructor from XML
//
Shader::Shader(const std::string& xmlfilepath)
{
	// Load the document
	XMLDocument document;
	XMLError error = document.LoadFile(xmlfilepath.c_str());

	if (error != tinyxml2::XML_SUCCESS)
	{
		ERROR_RECOVERABLE(Stringf("Error: Shader::LoadShadersFromXML couldn't load file \"%s\"", xmlfilepath.c_str()));
		return;
	}

	// I keep a root around just because I like having a single root element
	XMLElement* shaderElement = document.RootElement();

	ParseProgram(*shaderElement);
	ParseCullMode(*shaderElement);
	ParseFillMode(*shaderElement);
	ParseWindOrder(*shaderElement);
	ParseDepthMode(*shaderElement);
	ParseBlendMode(*shaderElement);
}


//-----------------------------------------------------------------------------------------------
// Constructor with custom render state
//
Shader::Shader(const RenderState& renderState, const ShaderProgram* program)
	: m_renderState(renderState)
	, m_shaderProgram(program)
{
}


//-----------------------------------------------------------------------------------------------
// Destructor - deleting the shader also deletes the ShaderProgram it currently has
//
Shader::~Shader()
{
	if (m_shaderProgram != nullptr)
	{
		delete m_shaderProgram;
		m_shaderProgram = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
// Returns a new copy of this shader
//
Shader* Shader::Clone()
{
	const ShaderProgram* program = m_shaderProgram->Clone();
	RenderState renderState = m_renderState;

	Shader* cloneShader = new Shader(renderState, program);

	return cloneShader;
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the shader program parameters and assigns the program of this shader
//
void Shader::ParseProgram(const XMLElement& shaderElement)
{
	const XMLElement* programElement = shaderElement.FirstChildElement("program");

	if (programElement != nullptr)
	{
		// Get the program name
		std::string programName = ParseXmlAttribute(*programElement, "name", "NO_PROGRAM_NAME_SPECIFIED_IN_XML");

		// Get the program data and build it
		const XMLElement* vsElement = programElement->FirstChildElement("vertex");
		const XMLElement* fsElement = programElement->FirstChildElement("fragment");

		if (vsElement != nullptr && fsElement != nullptr)
		{
			std::string vsFilepath = ParseXmlAttribute(*vsElement, "file");
			std::string fsFilepath = ParseXmlAttribute(*fsElement, "file");

			if (vsFilepath.size() > 0 && fsFilepath.size() > 0)
			{
				ShaderProgram* program = new ShaderProgram(programName);
				program->LoadProgramFromFiles(vsFilepath.c_str(), fsFilepath.c_str());	// Will assign invalid program internally if compilation fails

				m_shaderProgram = program;			
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the cull mode specified
//
void Shader::ParseCullMode(const XMLElement& shaderElement)
{
	const XMLElement* cullElement = shaderElement.FirstChildElement("cull");

	if (cullElement != nullptr)
	{
		std::string cullText = ParseXmlAttribute(*cullElement, "mode", "back");

		if		(cullText == "front")	{ m_renderState.m_cullMode = CULL_MODE_FRONT; }
		else if (cullText == "none")	{ m_renderState.m_cullMode = CULL_MODE_NONE;  }
		else
		{
			m_renderState.m_cullMode = CULL_MODE_BACK;	// Default to back culling
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the fill mode specified
//
void Shader::ParseFillMode(const XMLElement& shaderElement)
{
	const XMLElement* fillElement = shaderElement.FirstChildElement("fill");

	if (fillElement != nullptr)
	{
		std::string fillText = ParseXmlAttribute(*fillElement, "mode", "solid");

		if (fillText == "wire")	{ m_renderState.m_fillMode = FILL_MODE_WIRE; }
		else
		{
			m_renderState.m_fillMode = FILL_MODE_SOLID;	// Default to solid fill
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the wind order specified
//
void Shader::ParseWindOrder(const XMLElement& shaderElement)
{
	const XMLElement* windElement = shaderElement.FirstChildElement("wind");

	if (windElement != nullptr)
	{
		std::string windText = ParseXmlAttribute(*windElement, "order", "ccw");

		if (windText == "cw")	{ m_renderState.m_windOrder = WIND_CLOCKWISE; }
		else
		{
			m_renderState.m_windOrder = WIND_COUNTER_CLOCKWISE;	// Default to CCW rotation being front
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the depth mode and write flag specified
//
void Shader::ParseDepthMode(const XMLElement& shaderElement)
{
	const XMLElement* depthElement = shaderElement.FirstChildElement("depth");

	if (depthElement != nullptr)
	{
		std::string depthTest = ParseXmlAttribute(*depthElement, "test", "less");

		if		(depthTest == "lequal")		{ m_renderState.m_depthTest = DEPTH_TEST_LEQUAL; }
		else if (depthTest == "greater")	{ m_renderState.m_depthTest = DEPTH_TEST_GREATER; }
		else if (depthTest == "gequal")		{ m_renderState.m_depthTest = DEPTH_TEST_GEQUAL; }
		else if (depthTest == "equal")		{ m_renderState.m_depthTest = DEPTH_TEST_EQUAL; }
		else if (depthTest == "notequal")	{ m_renderState.m_depthTest = DEPTH_TEST_NOT_EQUAL; }
		else if (depthTest == "always")		{ m_renderState.m_depthTest = DEPTH_TEST_ALWAYS; }
		else if (depthTest == "never")		{ m_renderState.m_depthTest = DEPTH_TEST_NEVER; }
		else
		{
			m_renderState.m_depthTest = DEPTH_TEST_LESS;	// Default to less than test
		}
	}

	m_renderState.m_shouldWriteDepth = ParseXmlAttribute(*depthElement, "write", true);
}


//-----------------------------------------------------------------------------------------------
// Parses the shader xml element for the blend mode and blend factors specified
//
void Shader::ParseBlendMode(const XMLElement& shaderElement)
{
	const XMLElement* blendElement = shaderElement.FirstChildElement("blend");

	if (blendElement != nullptr)
	{
		// Color
		const XMLElement* colorElement = blendElement->FirstChildElement("color");

		if (colorElement != nullptr)
		{
			// Op mode
			std::string opText = ParseXmlAttribute(*colorElement, "op", "add");

			if		(opText == "subtract")			{ m_renderState.m_colorBlendOp = BLEND_OP_SUBTRACT; }
			else if (opText == "reverse_subtract")	{ m_renderState.m_colorBlendOp = BLEND_OP_REVERSE_SUBTRACT; }
			else if (opText == "min")				{ m_renderState.m_colorBlendOp = BLEND_OP_MIN; }
			else if (opText == "max")				{ m_renderState.m_colorBlendOp = BLEND_OP_MAX; }
			else
			{
				m_renderState.m_colorBlendOp = BLEND_OP_ADD;	// Default to add op
			}

			// Factors
			std::string srcText = ParseXmlAttribute(*colorElement, "source", "source_alpha");

			if		(srcText == "one")						{ m_renderState.m_colorSrcFactor = BLEND_FACTOR_ONE; }
			else if (srcText == "zero")						{ m_renderState.m_colorSrcFactor = BLEND_FACTOR_ZERO; }
			else if (srcText == "one_minus_source_alpha")	{ m_renderState.m_colorSrcFactor = BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA; }
			else
			{
				m_renderState.m_colorSrcFactor = BLEND_FACTOR_SOURCE_ALPHA;	// Default to source alpha
			}

			std::string destText = ParseXmlAttribute(*colorElement, "destination", "one_minus_source_alpha");

			if		(destText == "one")				{ m_renderState.m_colorDstFactor = BLEND_FACTOR_ONE; }
			else if (destText == "zero")			{ m_renderState.m_colorDstFactor = BLEND_FACTOR_ZERO; }
			else if (destText == "source_alpha")	{ m_renderState.m_colorDstFactor = BLEND_FACTOR_SOURCE_ALPHA; }
			else
			{
				m_renderState.m_colorDstFactor = BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA;	// Default to one minus source alpha
			}
		}

		// Alpha
		const XMLElement* alphaElement = blendElement->FirstChildElement("alpha");

		if (alphaElement != nullptr)
		{
			// Op mode
			std::string opText = ParseXmlAttribute(*alphaElement, "op", "add");

			if		(opText == "subtract")			{ m_renderState.m_alphaBlendOp = BLEND_OP_SUBTRACT; }
			else if (opText == "reverse_subtract")	{ m_renderState.m_alphaBlendOp = BLEND_OP_REVERSE_SUBTRACT; }
			else if (opText == "min")				{ m_renderState.m_alphaBlendOp = BLEND_OP_MIN; }
			else if (opText == "max")				{ m_renderState.m_alphaBlendOp = BLEND_OP_MAX; }
			else
			{
				m_renderState.m_alphaBlendOp = BLEND_OP_ADD;	// Default to add op
			}

			// Factors
			std::string srcText = ParseXmlAttribute(*alphaElement, "source", "source_alpha");

			if		(srcText == "one")						{ m_renderState.m_alphaSrcFactor = BLEND_FACTOR_ONE; }
			else if (srcText == "zero")						{ m_renderState.m_alphaSrcFactor = BLEND_FACTOR_ZERO; }
			else if (srcText == "one_minus_source_alpha")	{ m_renderState.m_alphaSrcFactor = BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA; }
			else
			{
				m_renderState.m_alphaSrcFactor = BLEND_FACTOR_SOURCE_ALPHA;	// Default to source alpha
			}

			std::string destText = ParseXmlAttribute(*alphaElement, "destination", "one_minus_source_alpha");

			if		(destText == "one")				{ m_renderState.m_alphaDstFactor = BLEND_FACTOR_ONE; }
			else if (destText == "zero")			{ m_renderState.m_alphaDstFactor = BLEND_FACTOR_ZERO; }
			else if (destText == "source_alpha")	{ m_renderState.m_alphaDstFactor = BLEND_FACTOR_SOURCE_ALPHA; }
			else
			{
				m_renderState.m_alphaDstFactor = BLEND_FACTOR_ONE_MINUS_SOURCE_ALPHA;	// Default to one minus source alpha
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
// Sets the shader program of this shader to the one specified
//
void Shader::SetProgram(ShaderProgram* program)
{
	m_shaderProgram = program;
}


//-----------------------------------------------------------------------------------------------
// Sets the cull mode on the shader
//
void Shader::SetCullMode(CullMode mode)
{
	m_renderState.m_cullMode = mode;
}


//-----------------------------------------------------------------------------------------------
// Sets the fill mode on the shader
//
void Shader::SetFillMode(FillMode mode)
{
	m_renderState.m_fillMode = mode;
}


//-----------------------------------------------------------------------------------------------
// Sets the wind order on the shader
//
void Shader::SetWindOrder(WindOrder order)
{
	m_renderState.m_windOrder = order;
}


//-----------------------------------------------------------------------------------------------
// Sets the depth test and write flag on the shader
//
void Shader::EnableDepth(DepthTest test, bool shouldWrite)
{
	m_renderState.m_depthTest = test;
	m_renderState.m_shouldWriteDepth = shouldWrite;
}


//-----------------------------------------------------------------------------------------------
// "Disables" depth on the shader by setting always draw and no write-to-depth flags
//
void Shader::DisableDepth()
{
	m_renderState.m_depthTest = DEPTH_TEST_ALWAYS;
	m_renderState.m_shouldWriteDepth = false;
}


//-----------------------------------------------------------------------------------------------
// Sets the blend op and factors for color on the shader
//
void Shader::EnableColorBlending(BlendOp op, BlendFactor srcFactor, BlendFactor dstFactor)
{
	m_renderState.m_colorBlendOp	= op;
	m_renderState.m_colorSrcFactor	= srcFactor;
	m_renderState.m_colorDstFactor	= dstFactor;
}


//-----------------------------------------------------------------------------------------------
// Sets the blend op and factors for alpha on the shader
//
void Shader::EnableAlphaBlending(BlendOp op, BlendFactor srcFactor, BlendFactor dstFactor)
{
	m_renderState.m_alphaBlendOp	= op;
	m_renderState.m_alphaSrcFactor	= srcFactor;
	m_renderState.m_alphaDstFactor	= dstFactor;
}


//-----------------------------------------------------------------------------------------------
// "Disables" blending for color on the shader by setting factors to not blend
//
void Shader::DisableColorBlending()
{
	m_renderState.m_colorBlendOp	= BLEND_OP_ADD;
	m_renderState.m_colorSrcFactor	= BLEND_FACTOR_ONE;
	m_renderState.m_colorDstFactor	= BLEND_FACTOR_ZERO;
}


//-----------------------------------------------------------------------------------------------
// "Disables" blending for alpha on the shader by setting factors to not blend
//
void Shader::DisableAlphaBlending()
{
	m_renderState.m_colorBlendOp	= BLEND_OP_ADD;
	m_renderState.m_colorSrcFactor	= BLEND_FACTOR_ONE;
	m_renderState.m_colorDstFactor	= BLEND_FACTOR_ONE;
}


//-----------------------------------------------------------------------------------------------
// Returns the program currently being used by the shader
//
const ShaderProgram* Shader::GetProgram() const
{
	return m_shaderProgram;
}


//-----------------------------------------------------------------------------------------------
// Returns the render state struct for the shader
//
const RenderState& Shader::GetRenderState() const
{
	return m_renderState;
}


//-----------------------------------------------------------------------------------------------
// Returns the draw layer index for this shader
//
unsigned int Shader::GetLayer() const
{
	return m_layer;
}


//-----------------------------------------------------------------------------------------------
// Returns the draw queue order of this shader
//
SortingQueue Shader::GetQueue() const
{
	return m_queue;
}


//-----------------------------------------------------------------------------------------------
// Builds and returns a shader given the shader source and render state
//
Shader* Shader::BuildShader(const std::string& name, const char* vsSource, const char* fsSource, 
	const RenderState& state, unsigned int sortingLayer, SortingQueue sortingQueue)
{
	ShaderProgram* program = new ShaderProgram(name);
	bool loadSuccessful = program->LoadProgramFromSources(vsSource, fsSource);

	// If default failed then assign it the invalid shader in the map
	if (!loadSuccessful)
	{
		program->LoadProgramFromSources(ShaderSource::INVALID_VS, ShaderSource::INVALID_FS);
	}

	Shader* shader = new Shader(state, program);
	shader->m_layer = sortingLayer;
	shader->m_queue = sortingQueue;

	return shader;
}
