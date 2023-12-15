#ifndef BALL_H
#define BALL_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "texture.hpp"

//Ball holds the state of the Ball object.
//Contains extra functionality specific to Breakout's ball object that
//are too specific for just GameObject
class BallObject : public GameObject {

public:
	//Ball state
	float Radius;
	bool Stuck;

	bool Sticky, PassThrough;

	//Constructors
	BallObject();
	BallObject(glm::vec2 pos, float radius, glm::vec2 velocity, Texture2D sprite);

	//Moves the ball, keeping it constrained within the window bounds (except the bottom edge); returns new position.
	glm::vec2 Move(float dt, unsigned int window_width);

	//Resets the ball to original state with given position and velocity
	void Reset(glm::vec2 position, glm::vec2 velocity);

};

#endif