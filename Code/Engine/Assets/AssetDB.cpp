/************************************************************************/
/* File: AssetDatabase.cpp
/* Author: Andrew Chase
/* Date: April 11th, 2018
/* Description: Implementation of the Resource class
/************************************************************************/
#include "Engine/Core/Image.hpp"
#include "Engine/Assets/AssetDB.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Assets/AssetCollection.hpp"
#include "Engine/Core/Time/ProfileScoped.hpp"
#include "Engine/Rendering/Shaders/Shader.hpp"
#include "Engine/Rendering/Resources/Skybox.hpp"
#include "Engine/Rendering/Resources/Texture.hpp"
#include "Engine/Rendering/Meshes/MeshBuilder.hpp"
#include "Engine/Rendering/Shaders/ShaderSource.hpp"
#include "Engine/Rendering/Resources/BitmapFont.hpp"
#include "Engine/Rendering/Resources/TextureCube.hpp"
#include "Engine/Rendering/Resources/SpriteSheet.hpp"
#include "Engine/Rendering/Resources/SpriteSheet.hpp"
#include "Engine/Rendering/Shaders/ShaderProgram.hpp"
#include "Engine/Core/DeveloperConsole/DevConsole.hpp"
#include "Engine/Rendering/Meshes/MeshGroupBuilder.hpp"
#include "Engine/Rendering/Materials/MaterialInstance.hpp"

//-----------------------------------------------------------------------------------------------
// Constructs all the built-in assets for the Engine, called at start up
//
void AssetDB::CreateBuiltInAssets()
{
	//--------------------Textures--------------------
	CreateTextures();

	//--------------------Shaders--------------------
	CreateShaders();

	//-------------------Materials-------------------
	CreateMaterials();

	//---------------------Meshes--------------------
	CreateMeshes();
}


//-----------------------------------------------------------------------------------------------
// Creates all built-in textures for the engine, and stores them in the AssetDB
//
void AssetDB::CreateTextures()
{
	Texture* whiteTexture = new Texture();
	whiteTexture->CreateFromImage(&Image::IMAGE_WHITE);
	AssetCollection<Texture>::AddAsset("White", whiteTexture);

	Texture* flatTexture = new Texture();
	flatTexture->CreateFromImage(&Image::IMAGE_FLAT);
	AssetCollection<Texture>::AddAsset("Flat", flatTexture);

	Texture* blackTexture = new Texture();
	blackTexture->CreateFromImage(&Image::IMAGE_BLACK);
	AssetCollection<Texture>::AddAsset("Black", blackTexture);

	Texture* defaultTexture = new Texture();
	defaultTexture->CreateFromImage(&Image::IMAGE_DEFAULT_TEXTURE);
	AssetCollection<Texture>::AddAsset("Default", defaultTexture);
}


//-----------------------------------------------------------------------------------------------
// Creates all built-in shaders for the engine, and stores them in the AssetDB
//
void AssetDB::CreateShaders()
{
	ShaderProgram* invalidProgram = new ShaderProgram(ShaderSource::INVALID_SHADER_NAME);
	bool loadSuccessful = invalidProgram->LoadProgramFromSources(ShaderSource::INVALID_VS, ShaderSource::INVALID_FS, true);
	ASSERT_OR_DIE(loadSuccessful, "Error: Renderer::CreateBuildInShaders() could not build the Invalid Shader.");

	// Get the invalid shader in first
	Shader* invalidShader = new Shader(ShaderSource::INVALID_RENDER_STATE, invalidProgram);
	AssetCollection<Shader>::AddAsset(ShaderSource::INVALID_SHADER_NAME, invalidShader);

	// Then construct all others, since if they fail to compile we assign them the invalid shader data
	Shader* defaultOpaque	= Shader::BuildShader(ShaderSource::DEFAULT_OPAQUE_NAME,	ShaderSource::DEFAULT_OPAQUE_VS,	ShaderSource::DEFAULT_OPAQUE_FS,	ShaderSource::DEFAULT_OPAQUE_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* defaultAlpha	= Shader::BuildShader(ShaderSource::DEFAULT_ALPHA_NAME,		ShaderSource::DEFAULT_ALPHA_VS,		ShaderSource::DEFAULT_ALPHA_FS,		ShaderSource::DEFAULT_ALPHA_STATE,		ShaderSource::DEFAULT_ALPHA_LAYER, ShaderSource::DEFAULT_ALPHA_QUEUE);
	Shader* uiShader		= Shader::BuildShader(ShaderSource::UI_SHADER_NAME,			ShaderSource::UI_SHADER_VS,			ShaderSource::UI_SHADER_FS,			ShaderSource::UI_SHADER_STATE,			ShaderSource::DEFAULT_ALPHA_LAYER, ShaderSource::DEFAULT_ALPHA_QUEUE);
	Shader* debugShader		= Shader::BuildShader(ShaderSource::DEBUG_RENDER_NAME,		ShaderSource::DEBUG_RENDER_VS,		ShaderSource::DEBUG_RENDER_FS,		ShaderSource::DEBUG_RENDER_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* phongOpaque		= Shader::BuildShader(ShaderSource::PHONG_OPAQUE_NAME,		ShaderSource::PHONG_OPAQUE_VS,		ShaderSource::PHONG_OPAQUE_FS,		ShaderSource::PHONG_OPAQUE_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* phongAlpha		= Shader::BuildShader(ShaderSource::PHONG_ALPHA_NAME,		ShaderSource::PHONG_ALPHA_VS,		ShaderSource::PHONG_ALPHA_FS,		ShaderSource::PHONG_ALPHA_STATE,		ShaderSource::DEFAULT_ALPHA_LAYER, ShaderSource::DEFAULT_ALPHA_QUEUE);
	Shader* vertexNormal	= Shader::BuildShader(ShaderSource::VERTEX_NORMAL_NAME,		ShaderSource::VERTEX_NORMAL_VS,		ShaderSource::VERTEX_NORMAL_FS,		ShaderSource::VERTEX_NORMAL_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* vertexTangent	= Shader::BuildShader(ShaderSource::VERTEX_TANGENT_NAME,	ShaderSource::VERTEX_TANGENT_VS,	ShaderSource::VERTEX_TANGENT_FS,	ShaderSource::VERTEX_TANGENT_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* vertexBitangent	= Shader::BuildShader(ShaderSource::VERTEX_BITANGENT_NAME,	ShaderSource::VERTEX_BITANGENT_VS,	ShaderSource::VERTEX_BITANGENT_FS,	ShaderSource::VERTEX_BITANGENT_STATE,	ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* surfaceNormal	= Shader::BuildShader(ShaderSource::SURFACE_NORMAL_NAME,	ShaderSource::SURFACE_NORMAL_VS,	ShaderSource::SURFACE_NORMAL_FS,	ShaderSource::SURFACE_NORMAL_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* worldNormal		= Shader::BuildShader(ShaderSource::WORLD_NORMAL_NAME,		ShaderSource::WORLD_NORMAL_VS,		ShaderSource::WORLD_NORMAL_FS,		ShaderSource::WORLD_NORMAL_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* diffuse			= Shader::BuildShader(ShaderSource::DIFFUSE_NAME,			ShaderSource::DIFFUSE_VS,			ShaderSource::DIFFUSE_FS,			ShaderSource::DIFFUSE_STATE,			ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* specular		= Shader::BuildShader(ShaderSource::SPECULAR_NAME,			ShaderSource::SPECULAR_VS,			ShaderSource::SPECULAR_FS,			ShaderSource::SPECULAR_STATE,			ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* lighting		= Shader::BuildShader(ShaderSource::LIGHTING_NAME,			ShaderSource::LIGHTING_VS,			ShaderSource::LIGHTING_FS,			ShaderSource::LIGHTING_STATE,			ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* uv				= Shader::BuildShader(ShaderSource::UV_NAME,				ShaderSource::UV_VS,				ShaderSource::UV_FS,				ShaderSource::UV_STATE,					ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* skybox			= Shader::BuildShader(ShaderSource::SKYBOX_SHADER_NAME,		ShaderSource::SKYBOX_SHADER_VS,		ShaderSource::SKYBOX_SHADER_FS,		ShaderSource::SKYBOX_SHADER_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);

	Shader* opaqueInstanced			= Shader::BuildShader(ShaderSource::DEFAULT_OPAQUE_INSTANCED_NAME,	ShaderSource::DEFAULT_OPAQUE_INSTANCED_VS,	ShaderSource::DEFAULT_OPAQUE_INSTANCED_FS,	ShaderSource::DEFAULT_OPAQUE_INSTANCED_STATE,		ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* alphaInstanced			= Shader::BuildShader(ShaderSource::DEFAULT_ALPHA_INSTANCED_NAME,	ShaderSource::DEFAULT_ALPHA_INSTANCED_VS,	ShaderSource::DEFAULT_ALPHA_INSTANCED_FS,	ShaderSource::DEFAULT_ALPHA_INSTANCED_STATE,		ShaderSource::DEFAULT_ALPHA_LAYER,	ShaderSource::DEFAULT_ALPHA_QUEUE);
	Shader* phongOpaqueInstanced	= Shader::BuildShader(ShaderSource::PHONG_OPAQUE_INSTANCED_NAME,	ShaderSource::PHONG_OPAQUE_INSTANCED_VS,	ShaderSource::PHONG_OPAQUE_INSTANCED_FS,	ShaderSource::PHONG_OPAQUE_INSTANCED_STATE,			ShaderSource::DEFAULT_OPAQUE_LAYER, ShaderSource::DEFAULT_OPAQUE_QUEUE);
	Shader* phongAlphaInstanced		= Shader::BuildShader(ShaderSource::PHONG_ALPHA_INSTANCED_NAME,		ShaderSource::PHONG_ALPHA_INSTANCED_VS,		ShaderSource::PHONG_ALPHA_INSTANCED_FS,		ShaderSource::PHONG_ALPHA_INSTANCED_STATE,			ShaderSource::DEFAULT_ALPHA_LAYER,	ShaderSource::DEFAULT_ALPHA_QUEUE);

	// Add the shaders in!
	AssetCollection<Shader>::AddAsset(ShaderSource::DEFAULT_OPAQUE_NAME,	defaultOpaque);
	AssetCollection<Shader>::AddAsset(ShaderSource::DEFAULT_ALPHA_NAME,		defaultAlpha);
	AssetCollection<Shader>::AddAsset(ShaderSource::UI_SHADER_NAME,			uiShader);
	AssetCollection<Shader>::AddAsset(ShaderSource::DEBUG_RENDER_NAME,		debugShader);
	AssetCollection<Shader>::AddAsset(ShaderSource::PHONG_OPAQUE_NAME,		phongOpaque);
	AssetCollection<Shader>::AddAsset(ShaderSource::PHONG_ALPHA_NAME,		phongAlpha);
	AssetCollection<Shader>::AddAsset(ShaderSource::VERTEX_NORMAL_NAME,		vertexNormal);
	AssetCollection<Shader>::AddAsset(ShaderSource::VERTEX_TANGENT_NAME,	vertexTangent);
	AssetCollection<Shader>::AddAsset(ShaderSource::VERTEX_BITANGENT_NAME,	vertexBitangent);
	AssetCollection<Shader>::AddAsset(ShaderSource::SURFACE_NORMAL_NAME,	surfaceNormal);
	AssetCollection<Shader>::AddAsset(ShaderSource::WORLD_NORMAL_NAME,		worldNormal);
	AssetCollection<Shader>::AddAsset(ShaderSource::DIFFUSE_NAME,			diffuse);
	AssetCollection<Shader>::AddAsset(ShaderSource::SPECULAR_NAME,			specular);
	AssetCollection<Shader>::AddAsset(ShaderSource::LIGHTING_NAME,			lighting);
	AssetCollection<Shader>::AddAsset(ShaderSource::UV_NAME,				uv);
	AssetCollection<Shader>::AddAsset(ShaderSource::SKYBOX_SHADER_NAME,		skybox);

	AssetCollection<Shader>::AddAsset(ShaderSource::DEFAULT_OPAQUE_INSTANCED_NAME,	opaqueInstanced);
	AssetCollection<Shader>::AddAsset(ShaderSource::DEFAULT_ALPHA_INSTANCED_NAME,	alphaInstanced);
	AssetCollection<Shader>::AddAsset(ShaderSource::PHONG_OPAQUE_INSTANCED_NAME,	phongOpaqueInstanced);
	AssetCollection<Shader>::AddAsset(ShaderSource::PHONG_ALPHA_INSTANCED_NAME,		phongAlphaInstanced);
}


//-----------------------------------------------------------------------------------------------
// Creates all built-in materials for the engine, and stores them in the AssetDB
//
void AssetDB::CreateMaterials()
{
	Material* debugMaterial = new Material("Debug_Render");
	debugMaterial->SetDiffuse(AssetDB::GetTexture("White"));
	debugMaterial->SetShader(AssetDB::GetShader(ShaderSource::DEBUG_RENDER_NAME));

	AssetCollection<Material>::AddAsset("Debug_Render", debugMaterial);

	Material* defaultMaterial = new Material("Default_Opaque");
	defaultMaterial->SetDiffuse(AssetDB::GetTexture("White"));
	defaultMaterial->SetShader(AssetDB::GetShader(ShaderSource::DEFAULT_OPAQUE_NAME));

	AssetCollection<Material>::AddAsset("Default_Opaque", defaultMaterial);

	Material* phongMaterial = new Material("Phong_Opaque");
	phongMaterial->SetDiffuse(GetTexture("Default"));
	phongMaterial->SetShader(GetShader(ShaderSource::PHONG_OPAQUE_NAME));

	AssetCollection<Material>::AddAsset("Phong_Opaque", phongMaterial);


	AssetCollection<Material>::AddAsset("Default_Opaque", defaultMaterial);

	Material* uiMat = new Material("UI");
	uiMat->SetDiffuse(AssetDB::GetTexture("White"));
	uiMat->SetShader(AssetDB::GetShader(ShaderSource::UI_SHADER_NAME));

	AssetCollection<Material>::AddAsset("UI", uiMat);

	Material* flChanMat = new Material("FLChan");
	flChanMat->SetDiffuse(CreateOrGetTexture("Data/Images/DevConsole/FLChan.png"));
	flChanMat->SetShader(AssetDB::GetShader(ShaderSource::UI_SHADER_NAME));

	AssetCollection<Material>::AddAsset("FLChan", flChanMat);



	Material* skyboxMat = new Material("Skybox");
	skyboxMat->SetDiffuse(AssetDB::GetTexture("White"));
	skyboxMat->SetShader(AssetDB::GetShader(ShaderSource::SKYBOX_SHADER_NAME));

	AssetCollection<Material>::AddAsset("Skybox", skyboxMat);
}


//-----------------------------------------------------------------------------------------------
// Creates all built-in meshes for the engine, and stores them in the AssetDB
//
void AssetDB::CreateMeshes()
{
	// Cube
	MeshBuilder mb;
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushCube(Vector3::ZERO, Vector3::ONES);
	mb.FinishBuilding();
	Mesh* cube = mb.CreateMesh();

	AssetCollection<Mesh>::AddAsset("Cube", cube);

	// Point
	mb.Clear();
	mb.BeginBuilding(PRIMITIVE_LINES, false);
	mb.PushPoint(Vector3::ZERO);
	mb.FinishBuilding();
	Mesh* point = mb.CreateMesh();

	AssetCollection<Mesh>::AddAsset("Point", point);

	// Sphere
	mb.Clear();
	mb.BeginBuilding(PRIMITIVE_TRIANGLES, true);
	mb.PushUVSphere(Vector3::ZERO, 1.f, 8, 4);
	mb.FinishBuilding();
	Mesh* sphere = mb.CreateMesh();

	AssetCollection<Mesh>::AddAsset("Sphere", sphere);

	// Bone
	mb.Clear();
	mb.BeginBuilding(PRIMITIVE_LINES, false);

	// Small "back" facing shape
	mb.PushLine(Vector3(0.f, 0.f, -0.5f), Vector3(0.f, 0.5f, 0.f));
	mb.PushLine(Vector3(0.f, 0.f, -0.5f), Vector3(0.f, -0.5f, 0.f));
	mb.PushLine(Vector3(0.f, 0.f, -0.5f), Vector3(0.5f, 0.f, 0.f));
	mb.PushLine(Vector3(0.f, 0.f, -0.5f), Vector3(-0.5f, 0.f, 0.f));

	// Larget "forward" shape
	mb.PushLine(Vector3(0.f, 0.5f, 0.f), Vector3(0.f, 0.f, 2.f));
	mb.PushLine(Vector3(0.f, -0.5f, 0.f), Vector3(0.f, 0.f, 2.f));
	mb.PushLine(Vector3(0.5f, 0.f, 0.f), Vector3(0.f, 0.f, 2.f));
	mb.PushLine(Vector3(-0.5f, 0.f, 0.f), Vector3(0.f, 0.f, 2.f));

	// Line going right
	mb.PushLine(Vector3::ZERO, Vector3(0.5f, 0.f, 0.f), Rgba::RED);

	// Line going up
	mb.PushLine(Vector3::ZERO, Vector3(0.f, 0.5f, 0.f), Rgba::GREEN);

	// Line through the center
	mb.PushLine(Vector3(0.f, 0.f, -0.5f), Vector3(0.f, 0.f, 2.f), Rgba::BLUE);

	mb.FinishBuilding();
	Mesh* boneMesh = mb.CreateMesh<Vertex3D_PCU>();

	AssetCollection<Mesh>::AddAsset("Bone", boneMesh);
}


//-----------------------------------------------------------------------------------------------
// Returns the image given by the filepath, returning null if it doesn't exist
//
Image* AssetDB::GetImage(const std::string& filename)
{
	return AssetCollection<Image>::GetAsset(filename);
}


//-----------------------------------------------------------------------------------------------
// Returns the image given by the filepath, attempting to construct it if it doesn't exist
//
Image* AssetDB::CreateOrGetImage(const std::string& filepath)
{
	Image* img = AssetCollection<Image>::GetAsset(filepath);

	if (img == nullptr)
	{
		img = new Image();
		bool successful = img->LoadFromFile(filepath);

		// Don't put nullptr in the AssetDB - allows for image reloading if failed
		if (!successful)
		{
			delete img;
			return nullptr;
		}

		AssetCollection<Image>::AddAsset(filepath, img);
	}

	return img;
}


//-----------------------------------------------------------------------------------------------
// Returns the texture given by the filepath, returning null if it doesn't exist
//
Texture* AssetDB::GetTexture(const std::string& filename)
{
	Texture* texture = AssetCollection<Texture>::GetAsset(filename);
	return texture;
}


//-----------------------------------------------------------------------------------------------
// Returns the Texture given by filepath, attempting to construct it if it doesn't exist
//
Texture* AssetDB::CreateOrGetTexture(const std::string& filepath, bool generateMipMaps /*= false*/)
{
	Texture* texture = AssetCollection<Texture>::GetAsset(filepath);

	if (texture == nullptr)
	{
		texture = new Texture();
		bool successful = texture->CreateFromFile(filepath, generateMipMaps);
		
		if (!successful)
		{
			delete texture;
			return nullptr;
		}

		AssetCollection<Texture>::AddAsset(filepath, texture);
	}

	return texture;
}


//-----------------------------------------------------------------------------------------------
// Returns the TextureCube given by name, returning null if it doesn't exist
//
TextureCube* AssetDB::GetTextureCube(const std::string& filename)
{
	return AssetCollection<TextureCube>::GetAsset(filename);
}


//-----------------------------------------------------------------------------------------------
// Returns the TextureCube given by name, attempting to create it if it doesn't exist
//
TextureCube* AssetDB::CreateOrGetTextureCube(const std::string& filepath)
{
	TextureCube* textureCube = AssetCollection<TextureCube>::GetAsset(filepath);

	if (textureCube == nullptr)
	{
		textureCube = new TextureCube();
		textureCube->CreateFromFile(filepath);
		AssetCollection<TextureCube>::AddAsset(filepath, textureCube);
	}

	return textureCube;
}


//-----------------------------------------------------------------------------------------------
// Returns the Skybox given by name, returning null if it doesn't exist
//
Skybox* AssetDB::GetSkybox(const std::string& textureName)
{
	return AssetCollection<Skybox>::GetAsset(textureName);
}


//-----------------------------------------------------------------------------------------------
// Returns the Skybox given by name, attempting to construct it if it doesn't exist
//
Skybox* AssetDB::CreateOrGetSkybox(const std::string& textureName)
{
	Skybox* skybox = AssetCollection<Skybox>::GetAsset(textureName);

	if (skybox == nullptr)
	{
		TextureCube* skyboxTexture = CreateOrGetTextureCube(textureName);
		skybox = new Skybox(skyboxTexture);

		AssetCollection<Skybox>::AddAsset(textureName, skybox);
	}

	return skybox;
}


//-----------------------------------------------------------------------------------------------
// Returns the SpriteSheet given by name, returning null if it doesn't exist
//
SpriteSheet* AssetDB::GetSpriteSheet(const std::string& name)
{
	return AssetCollection<SpriteSheet>::GetAsset(name);
}


//-----------------------------------------------------------------------------------------------
// Returns the SpriteSheet given by name, attempting to construct it if it doesn't exist
//
SpriteSheet* AssetDB::CreateOrGetSpriteSheet(const std::string& spritesheetPath)
{
	SpriteSheet* spritesheet = AssetCollection<SpriteSheet>::GetAsset(spritesheetPath);

	if (spritesheet == nullptr)
	{
		spritesheet = SpriteSheet::LoadSpriteSheet(spritesheetPath);
		AssetCollection<SpriteSheet>::AddAsset(spritesheetPath, spritesheet);
	}

	return spritesheet;
}


//-----------------------------------------------------------------------------------------------
// Returns the BitmapFont given by name, returning null if it doesn't exist
//
BitmapFont* AssetDB::GetBitmapFont(const std::string& filename)
{
	BitmapFont* font = AssetCollection<BitmapFont>::GetAsset(filename);
	return font;
}


//-----------------------------------------------------------------------------------------------
// Returns the BitmapFont given by name, attempting to construct it if it doesn't exist
//
BitmapFont* AssetDB::CreateOrGetBitmapFont(const std::string& fontPath)
{
	BitmapFont* font = AssetCollection<BitmapFont>::GetAsset(fontPath);
	
	if (font == nullptr)
	{
		// Make both the texture and spritesheet for the font here - don't add them to the database

		// Create the texture differently - it's outside the texture folder
		Texture* fontTexture = new Texture();
		fontTexture->CreateFromFile(fontPath);

		SpriteSheet spriteSheet = SpriteSheet(*fontTexture, IntVector2(16, 16));
		font = new BitmapFont(spriteSheet, 1.0f);

		AssetCollection<BitmapFont>::AddAsset(fontPath, font);
	}

	return font;
}


//-----------------------------------------------------------------------------------------------
// Returns the shared Mesh given by filename, returning null if it doesn't exist
//
Mesh* AssetDB::GetMesh(const std::string& filename)
{
	return AssetCollection<Mesh>::GetAsset(filename);
}


//-----------------------------------------------------------------------------------------------
// Returns the Mesh given by name, attempting to construct it if it doesn't exist
//
Mesh* AssetDB::CreateOrGetMesh(const std::string& meshPath)
{
	Mesh* mesh = AssetCollection<Mesh>::GetAsset(meshPath);

	if (mesh == nullptr)
	{
		MeshBuilder mb;
		mb.LoadFromObjFile(meshPath);
		mesh = mb.CreateMesh();
		AssetCollection<Mesh>::AddAsset(meshPath, mesh);
	}

	return mesh;
}


//-----------------------------------------------------------------------------------------------
// Adds the given mesh to the mesh collection
//
void AssetDB::AddMesh(const std::string& name, Mesh* mesh)
{
	Mesh* existingMesh = AssetCollection<Mesh>::GetAsset(name);
	ASSERT_OR_DIE(existingMesh == nullptr, Stringf("Error: AssetDB::AddMesh() tried to add a duplicate mesh of name \"%s\"", name.c_str()));

	AssetCollection<Mesh>::AddAsset(name, mesh);
}


//-----------------------------------------------------------------------------------------------
// Returns the Mesh Group given by filename, returning null if it doesn't exist
//
MeshGroup* AssetDB::GetMeshGroup(const std::string& filename)
{
	return AssetCollection<MeshGroup>::GetAsset(filename);
}


//-----------------------------------------------------------------------------------------------
// Returns the Mesh Group given by filename, returning null if it doesn't exist
//
MeshGroup* AssetDB::CreateOrGetMeshGroup(const std::string& filepath)
{
	MeshGroup* group = AssetCollection<MeshGroup>::GetAsset(filepath);

	if (group == nullptr)
	{
		MeshGroupBuilder mgb;
		mgb.LoadFromObjFile(filepath);
		group = mgb.CreateMeshGroup();
		AssetCollection<MeshGroup>::AddAsset(filepath, group);
	}

	return group;
}


//-----------------------------------------------------------------------------------------------
// Returns the Shader given by filename, returning null if it doesn't exist
//
Shader* AssetDB::GetShader(const std::string& name)
{
	return AssetCollection<Shader>::GetAsset(name);
}


//-----------------------------------------------------------------------------------------------
// Returns the Shader given by filename, attempting to construct it if it doesn't exist
//
Shader* AssetDB::CreateOrGetShader(const std::string& shaderPath)
{
	Shader* shader = AssetCollection<Shader>::GetAsset(shaderPath);

	if (shader == nullptr)
	{
		shader = new Shader(shaderPath);
		AssetCollection<Shader>::AddAsset(shaderPath, shader);
	}

	return shader;
}


//-----------------------------------------------------------------------------------------------
// Reloads and compiles all shader programs from file
//
void AssetDB::ReloadShaderPrograms()
{
	// Get an iterator over the shader collection
	std::map<std::string, Shader*> shaders = AssetCollection<Shader>::s_collection;
	std::map<std::string, Shader*>::iterator shaderItr = shaders.begin();

	for(shaderItr; shaderItr != shaders.end(); shaderItr++)
	{
		// Check to ensure that we don't attempt to load a built-in shader
		ShaderProgram* currProgram = shaderItr->second->GetProgram();
		if (currProgram->WasBuiltFromSource())
		{
			continue;
		}

		// Attempt the reload, will assign invalid shader if broken
		currProgram->LoadProgramFromFiles(currProgram->GetVSFilePathOrSource().c_str(), currProgram->GetFSFilePathOrSource().c_str());
	}

	ConsolePrintf(Rgba::GREEN, "ShaderPrograms reloaded successfully");
}


//-----------------------------------------------------------------------------------------------
// Returns the shared material given by name, or nullptr if it doesn't exist
//
Material* AssetDB::GetSharedMaterial(const std::string& name)
{
	Material* material = AssetCollection<Material>::GetAsset(name);
	return material;
}


//-----------------------------------------------------------------------------------------------
// Constructs a material instance of the material given by name
// Attempts to load and create the shared material if it doesn't exist
//
MaterialInstance* AssetDB::CreateMaterialInstance(const std::string& name)
{
	Material* sharedMaterial = CreateOrGetSharedMaterial(name);

	if (sharedMaterial != nullptr)
	{
		MaterialInstance* instance = new MaterialInstance(sharedMaterial);
		return instance;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
// Returns the shared material given by name, attempting to load and create it if if doesn't exist
//
Material* AssetDB::CreateOrGetSharedMaterial(const std::string& materialPath)
{
	Material* material = AssetCollection<Material>::GetAsset(materialPath);

	if (material == nullptr)
	{
		material = new Material(materialPath);
		bool success = material->LoadFromFile(materialPath);

		if (!success)
		{
			delete material;
			return nullptr;
		}

		AssetCollection<Material>::AddAsset(materialPath, material);
	}

	return material;
}
