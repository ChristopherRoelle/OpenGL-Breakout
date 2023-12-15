#include "resource_manager.hpp"

#include <iostream>
#include <sstream>
#include <fstream>

#include "stb_image.h"

//Instantiate static variables
std::map<std::string, Texture2D> ResourceManager::Textures;
std::map<std::string, Shader> ResourceManager::Shaders;

Shader ResourceManager::LoadShader(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile, std::string name) {
	Shaders[name] = LoadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name) {
	return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char* file, bool alpha, std::string name) {
	Textures[name] = LoadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear() {
	//Properly delete all shaders
	for (auto iter : Shaders) {
		glDeleteProgram(iter.second.ID);
	}

	//properly delete all textures
	for (auto iter : Textures) {
		glDeleteTextures(1, &iter.second.ID);
	}
}

Shader ResourceManager::LoadShaderFromFile(const char* vShaderFile, const char* fShaderFile, const char* gShaderFile) {

	//1. retrieve the vert/frag source from filepath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;

	try {
		//Open files
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);
		std::stringstream vShaderStream, fShaderStream;

		//read the file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();

		//Close the file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();

		//convert the stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();

		//if geometry shader path is present, do the same
		if (gShaderFile != nullptr) {
			std::ifstream geometryShaderFile(gShaderFile);
			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception e) {
		std::cout << "ERROR::SHADER: Failed to read shader files" << std::endl;
	}

	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();

	//Create Shader Object from the source
	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;
}

Texture2D ResourceManager::LoadTextureFromFile(const char* file, bool alpha) {

	//Create Texture Object
	Texture2D texture;

	if (alpha) {
		texture.Internal_Format = GL_RGBA;
		texture.Image_Format = GL_RGBA;
	}

	//load image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

	//now generate the texture
	texture.Generate(width, height, data);

	//Free the image data
	stbi_image_free(data);
	return texture;
}