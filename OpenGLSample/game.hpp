#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "game_level.hpp"
#include "Ball.hpp"
#include "PowerUp.hpp"

//Represents the current state of the Game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

//Direction Ball Collided on
enum Direction {
	UP, RIGHT, DOWN, LEFT
};

//Collision Tuple
typedef std::tuple<bool, Direction, glm::vec2> Collision; // <Collision? What Direction, difference vector center - closest point>

//Initial size of the player's paddle
const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);

//Initial velocity of the player's paddle
const float PLAYER_VELOCITY(500.0f);

//Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const float INITIAL_BALL_RADIUS = 12.5f;

//Holds all game-related state and functionality.
//Combines all game-related data in a single class
//for easy access to each component.
class Game
{

public:
	//Game Data
	GameState State;
	bool Keys[1024];
	bool KeysProcessed[1024];
	unsigned int Width, Height;

	std::vector<PowerUp> PowerUps;
	std::vector<GameLevel> Levels;
	unsigned int Level;

	unsigned int Lives;

	//Constructor/Destructor
	Game(unsigned int width, unsigned int height);
	~Game();

	//Initialize the game state (load all shaders/textures/levels)
	void Init();

	//Game loop
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();

	//Collisions
	void DoCollisions();

	//Reset
	void ResetLevel();
	void ResetPlayer();

	//PowerUps
	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
};

#endif

