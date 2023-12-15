#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>

//Texture2D is able to store and configure a texture in OpenGL.
//It also hosts utility functions for easy management.
class Texture2D {
	
public:
	//ID of texture Obj. used for all texture ops to ref to this particular texture
	unsigned int ID;

	//Image dimensions
	unsigned int Width, Height;

	//Format
	unsigned int Internal_Format; //Format of texture OBJ
	unsigned int Image_Format; //Format of the loaded image

	//Texture Config
	unsigned int Wrap_U; //Wrapping on U Axis
	unsigned int Wrap_V; //Wrapping on V Axis
	unsigned int Filter_Min; //filtering mode if texture pixels < screen pixels
	unsigned int Filter_Max; //filtering mode if texture pixels > screen pixels

	//Constructor (default texture modes)
	Texture2D();

	//Generate texture from image data
	void Generate(unsigned int width, unsigned int height, unsigned char* data);

	//Bind the texture as the current active GL_TEXTURE_2D obj
	void Bind() const;

};

#endif

