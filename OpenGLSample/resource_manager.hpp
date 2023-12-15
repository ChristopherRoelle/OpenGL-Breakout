#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <map>
#include <string>

#include <glad/glad.h>

#include "texture.hpp"
#include "shader.hpp"

//A static singleton ResourceManager class that hosts several
//functions to load Textures/Shaders. Each loaded texture/shader
//is also stored for future reference by string handles.
//All functions/resources are static and no public constructor defined.
class ResourceManager {
public:
	//resource storage
	static std::map<std::string, Shader> Shaders;
	static std::map<std::string, Texture2D> Textures;

	//loads (and generates) a shader program from file. Can load vert, frag, and geo
	static Shader LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name);

	//Retrieve a stored shader
	static Shader GetShader(std::string name);

	//load and gen texture from file.
	static Texture2D LoadTexture(const char* file, bool alpha, std::string name);

	//retrieve stored texture
	static Texture2D GetTexture(std::string name);

	//De-allocate all loaded resources
	static void Clear();

private:
	//private constructor, this is static
	ResourceManager();

	//Load and Gen a Shader from file
	static Shader LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile = nullptr);

	//Load a single texture from file
	static Texture2D LoadTextureFromFile(const char* file, bool alpha);
};

#endif