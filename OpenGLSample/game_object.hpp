#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "sprite_renderer.hpp"

//Container obj for holding all state relevant data for a single
//game object entity. Each object in the game likely needs the minimal
//state as described within GameObject.
class GameObject {

public:
	//Obj State
	glm::vec2 Position, Size, Velocity;
	glm::vec3 Color;

	float Rotation;
	bool IsSolid;
	bool Destroyed;

	unsigned int Lives;

	//Render State
	Texture2D Sprite;

	//Constructors
	GameObject();
	GameObject(glm::vec2 pos, glm::vec2 size, Texture2D sprite, glm::vec3 color = glm::vec3(1.0f), glm::vec2 velocity = glm::vec2(0.0f));

	//Draw Sprite
	virtual void Draw(SpriteRenderer& renderer);

};


#endif
