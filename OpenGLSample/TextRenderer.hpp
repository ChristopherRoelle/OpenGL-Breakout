#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <map>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"

//Character state info
struct Character {
	unsigned int TextureID; //ID handle of glyph texture
	glm::ivec2 Size;		//Size of Glyph
	glm::ivec2 Bearing;		//offset from baseline to left/top of glyph
	unsigned int Advance;	//horizontal offset to advance to next glyph
};

//A renderer class for rendering text displayed by a font using the FreeType library.
class TextRenderer
{
public:
	//Hold list of pre-compiled characters
	std::map<char, Character> Characters;

	//Shader used for Text Rendering
	Shader TextShader;

	//Constructor
	TextRenderer(unsigned int width, unsigned int height);

	//Pre-compiles a list of characters from a font sheet
	void Load(std::string font, unsigned int fontSize);

	//Renders a string of text using the precompiled list of characters
	void RenderText(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1.0f));

private:
	//Render state
	unsigned int VAO, VBO;
};

#endif