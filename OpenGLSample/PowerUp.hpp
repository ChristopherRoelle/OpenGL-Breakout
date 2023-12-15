#ifndef POWER_UP_H
#define POWER_UP_H

#include <string>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"

//The size of a powerup block
const glm::vec2 POWERUP_SIZE(60.0f, 20.0f);

//Velocity of a Powerup Block when spawned
const glm::vec2 VELOCITY(0.0f, 150.0f);

//Powerup inherits its state and rendering functions from
//GameObject but has extra info to state its active duration,
//whether it is active or not
//The type of powerup is stored as a string
class PowerUp : public GameObject {

public:
	//Powerup State
	std::string Type;

	float Duration;

	bool Activated;

	//Constructor
	PowerUp(std::string type, glm::vec3 color, float duration, glm::vec2 position, Texture2D texture)
		: GameObject(position, POWERUP_SIZE, texture, color, VELOCITY), Type(type), Duration(duration), Activated() {}
};

#endif
