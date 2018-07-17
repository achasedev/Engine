/************************************************************************/
/* File: AssetDatabase.hpp
/* Author: Andrew Chase
/* Date: April 11th, 2018
/* Description: Class to represent the interface for creating and getting
				all game and engine assets
/************************************************************************/
#pragma once
#include <vector>
#include <string>

class Mesh;
class Image;
class Shader;
class Skybox;
class Texture;
class Material;
class MeshGroup;
class BitmapFont;
class Renderable;
class SpriteSheet;
class TextureCube;
class Skeleton;
class ShaderProgram;
class MaterialInstance;

class AssetDB
{
public:
	//-----Public Methods-----

	static void CreateBuiltInAssets();
		static void CreateTextures();
		static void CreateShaders();
		static void CreateMaterials();
		static void CreateMeshes();

	// Images
	static Image* GetImage(const std::string& filename);
	static Image* CreateOrGetImage(const std::string& filename);

	// Textures
	static Texture* GetTexture(const std::string& filename);
	static Texture* CreateOrGetTexture(const std::string& filename, bool generateMipMaps = false);
	
	// Texture Cubes
	static TextureCube* GetTextureCube(const std::string& filename);
	static TextureCube* CreateOrGetTextureCube(const std::string& filename);

	// Skyboxes
	static Skybox* GetSkybox(const std::string& textureName);
	static Skybox* CreateOrGetSkybox(const std::string& textureName);

	// SpriteSheets
	static SpriteSheet* GetSpriteSheet(const std::string& name);
	static SpriteSheet* CreateOrGetSpriteSheet(const std::string& name);

	// Fonts
	static BitmapFont* GetBitmapFont(const std::string& filename);
	static BitmapFont* CreateOrGetBitmapFont(const std::string& filename);

	// Meshes
	static Mesh*	GetMesh(const std::string& filename);
	static Mesh*	CreateOrGetMesh(const std::string& filename);
	static void		AddMesh(const std::string& name, Mesh* mesh);

	// Mesh Groups
	static MeshGroup* GetMeshGroup(const std::string& filename);
	static MeshGroup* CreateOrGetMeshGroup(const std::string& filename);

	// Shaders
	static Shader*	GetShader(const std::string& name);
	static Shader*	CreateOrGetShader(const std::string& name);
	static void		ReloadShaderPrograms();
	
	static Material*			GetSharedMaterial(const std::string& name);
	static MaterialInstance*	CreateMaterialInstance(const std::string& name);
	static Material*			CreateOrGetSharedMaterial(const std::string& name);


private:
	//-----Private Data-----

};
