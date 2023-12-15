#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "sprite_renderer.hpp"
#include "shader.hpp"

//Hosts all Post Processing effects for Breakout.
//Renders out to a textured quad after which one can enable
//specific effects by enabling the booleans.
//Must call BeginRender() before rendering
//Must call EndRedner() after Rendering
class PostProcessor {

public:
	//State
	Shader PostProcessingShader;
	Texture2D Texture;

	unsigned int Width, Height;

	//Options
	bool Confuse, Chaos, Shake;

	//Constructor
	PostProcessor(Shader shader, unsigned int width, unsigned int height);

	//Prepare the Post Processor's Framebuffer Ops before rendering
	void BeginRender();

	//Call after rendering the game, stores all rendered data into a texture object
	void EndRender();

	//Render the PostProcessor texture Quad
	void Render(float time);

private:
	//Render state
	unsigned int MSFBO, FBO; //MSBO - Multisampled FBO.
	unsigned int RBO; //RBO - Multisampled color buffer
	unsigned int VAO;

	//Init Quad
	void InitRenderData();
};


#endif
