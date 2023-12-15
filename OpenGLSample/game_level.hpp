#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "game_object.hpp"
#include "sprite_renderer.hpp"
#include "resource_manager.hpp"

//Game Level holds all Tiles as part of a Breakout level and
//hosts functionality to Load/Render levels from the hard disk.
class GameLevel {

public:
	//level state
	std::vector<GameObject> Bricks;

	//Constructor
	GameLevel() {};

	//Load level from file
	void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);

	//Render level
	void Draw(SpriteRenderer& renderer);

	//Check if the level is complete (all non-solid bricks are destroyed)
	bool IsCompleted();

private:
	//Instatiate the level
	void Init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);

};

#endif