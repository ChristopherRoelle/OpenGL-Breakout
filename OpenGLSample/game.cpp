#include "game.hpp"
#include "resource_manager.hpp"
#include "sprite_renderer.hpp"
#include "game_object.hpp"
#include "Ball.hpp"
#include "ParticleGenerator.hpp"
#include "PostProcessor.hpp"

#include <sstream>

//Audio
#include <irrKlang.h>
#include "TextRenderer.hpp"
using namespace irrklang;

// Game-related State data
SpriteRenderer* Renderer;
GameObject* Player;
BallObject* Ball;
ParticleGenerator* Particles;
PostProcessor* Effects;

//Text
TextRenderer* Text;

//Audio
ISoundEngine* SoundEngine = createIrrKlangDevice();

//Effects
float ShakeTime = 0.0f;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), KeysProcessed(), Width(width), Height(height), Level(0), Lives(3)
{
    //Player->Lives = Lives;
}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Particles;
    delete Effects;
    delete Text;
    SoundEngine->drop();
}

void Game::Init()
{

    //Audio
    SoundEngine->play2D("audio/breakout.mp3", true);

    //Text
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("fonts/ocraext.TTF", 24);

    // load shaders
    ResourceManager::LoadShader("shaders/spriteVertex.glsl", "shaders/spriteFragment.glsl", nullptr, "sprite");
    ResourceManager::LoadShader("shaders/particleVertex.glsl", "shaders/particleFragment.glsl", nullptr, "particle");
    ResourceManager::LoadShader("shaders/postProcessingVertex.glsl", "shaders/postProcessingFragment.glsl", nullptr, "post");

    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
        static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
    ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

    // load textures
    ResourceManager::LoadTexture("textures/background.jpg", false, "background");
    ResourceManager::LoadTexture("textures/ball.png", true, "ball");
    ResourceManager::LoadTexture("textures/block.png", false, "block");
    ResourceManager::LoadTexture("textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");
    ResourceManager::LoadTexture("textures/particle.png", true, "particle");

    //Powerups
    ResourceManager::LoadTexture("textures/powerup_speedUp.png", true, "speed-up");
    ResourceManager::LoadTexture("textures/powerup_speedDown.png", true, "speed-down");
    ResourceManager::LoadTexture("textures/powerup_sticky.png", true, "sticky");
    ResourceManager::LoadTexture("textures/powerup_passthrough.png", true, "pass-through");
    ResourceManager::LoadTexture("textures/powerup_increase.png", true, "pad-increase");
    ResourceManager::LoadTexture("textures/powerup_decrease.png", true, "pad-decrease");
    ResourceManager::LoadTexture("textures/powerup_confuse.png", true, "confuse");
    ResourceManager::LoadTexture("textures/powerup_chaos.png", true, "chaos");
    ResourceManager::LoadTexture("textures/powerup_lifeUp.png", true, "life-up");

    // set render-specific controls
    Shader shader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(shader);
    shader = ResourceManager::GetShader("particle");
    Particles = new ParticleGenerator(shader, ResourceManager::GetTexture("particle"), 500);
    shader = ResourceManager::GetShader("post");
    Effects = new PostProcessor(shader, this->Width, this->Height);

    // load levels
    GameLevel one; one.Load("levels/one.lvl", this->Width, this->Height / 2);
    GameLevel two; two.Load("levels/two.lvl", this->Width, this->Height / 2);
    GameLevel three; three.Load("levels/three.lvl", this->Width, this->Height / 2);
    GameLevel four; four.Load("levels/four.lvl", this->Width, this->Height / 2);
    GameLevel five; five.Load("levels/five.lvl", this->Width, this->Height / 2);
    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Levels.push_back(four);
    this->Levels.push_back(five);
    this->Level = 0;

    // configure game objects
    glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
    Player->Lives = Lives;
    glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - INITIAL_BALL_RADIUS, -INITIAL_BALL_RADIUS * 2.0f);
    
    Ball = new BallObject(ballPos, INITIAL_BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("ball"));
    Ball->Color = glm::vec3(1.0f, 0.0f, 0.0f);
}

void Game::Update(float dt)
{
    // update objects
    Ball->Move(dt, this->Width);

    // check for collisions
    this->DoCollisions();

    // update particles
    Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

    // check loss
    if (Ball->Position.y >= this->Height) // did ball reach bottom edge?
    {
        --Player->Lives;

        //Did the player lose all lives? Game over
        if (Player->Lives == 0)
        {
            this->ResetLevel();
            this->State = GAME_MENU;
        }
        this->ResetPlayer();
    }

    //Update Effects
    if (ShakeTime > 0.0f) {
        ShakeTime -= dt;

        if (ShakeTime <= 0.0f) {
            Effects->Shake = false;
        }
    }

    //Check Win
    if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
    {
        this->ResetLevel();
        this->ResetPlayer();
        Effects->Chaos = true;
        this->State = GAME_WIN;
    }

    //Update Powerups
    UpdatePowerUps(dt);
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        // move player paddle
        if (this->Keys[GLFW_KEY_A])
        {
            if (Player->Position.x >= 0.0f)
            {
                Player->Position.x -= velocity;
                if (Ball->Stuck)
                    Ball->Position.x -= velocity;
            }
        }
        if (this->Keys[GLFW_KEY_D])
        {
            if (Player->Position.x <= this->Width - Player->Size.x)
            {
                Player->Position.x += velocity;
                if (Ball->Stuck)
                    Ball->Position.x += velocity;
            }
        }
        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;
    }

    //Game Menu Input
    if (this->State == GAME_MENU) 
    {
        //Enter a level
        if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
        }

        //Navigate Levels
        if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
        {
            this->Level = (this->Level + 1) % this->Levels.size();
            this->KeysProcessed[GLFW_KEY_W] = true;
        }

        if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
        {
            this->KeysProcessed[GLFW_KEY_S] = true;
            if (this->Level > 0)
                --this->Level;
            else
                this->Level = this->Levels.size() - 1;
        }
    }

    //Game Win Input
    if (this->State == GAME_WIN) 
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->KeysProcessed[GLFW_KEY_ENTER] = true;
            Effects->Chaos = false;
            this->State = GAME_MENU;
        }
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE || this->State == GAME_MENU || this->State == GAME_WIN)
    {
        //Post Processor start
        Effects->BeginRender();

        // draw background
        Texture2D background = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(background, glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        // draw level
        this->Levels[this->Level].Draw(*Renderer);

        // draw player
        Player->Draw(*Renderer);

        // draw particles	
        Particles->Draw();

        //Draw Powerups
        for (PowerUp& powerUp : this->PowerUps) {
            if (!powerUp.Destroyed) {
                powerUp.Draw(*Renderer);
            }
        }

        // draw ball
        Ball->Draw(*Renderer);

        //Post Processor end
        Effects->EndRender();
        Effects->Render(glfwGetTime());

        //Draw UI (No Post)
        std::stringstream ss; ss << Player->Lives;
        Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
    }

    //Game Menu only text
    if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
        Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
    }

    //Game Win render
    if (this->State == GAME_WIN) {
        Text->RenderText("You WON!!!", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0));
        Text->RenderText("Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0));
    }
}


void Game::ResetLevel()
{
    //Remove all powerups on screen
    for (PowerUp& powerUp : this->PowerUps) {
        powerUp.Destroyed = true;
    }

    //Load a level
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height / 2); //Standard
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height / 2); //Lockout
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height / 2); //Space Invader
    else if (this->Level == 3)
        this->Levels[3].Load("levels/four.lvl", this->Width, this->Height / 2); //holes
    else if (this->Level == 4)
        this->Levels[4].Load("levels/five.lvl", this->Width, this->Height / 2); //holes

    //Set Player's lives
    Player->Lives = 3;
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
    Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - INITIAL_BALL_RADIUS, -(INITIAL_BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

    //disable all active powerups
    Effects->Chaos = Effects->Confuse = false;
    Ball->PassThrough = Ball->Sticky = false;
    Player->Color = glm::vec3(1.0f);
    Ball->Color = glm::vec3(1.0f, 0.0f, 0.0f);
}

//Prototypes
bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 closest);
bool ShouldSpawn(unsigned int chance);
void ActivatePowerUp(PowerUp& powerUp);
bool IsOtherPowerUpActive(std::vector <PowerUp>& powerUps, std::string type);

// collision detection
void Game::DoCollisions()
{
    for (GameObject& box : this->Levels[this->Level].Bricks)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid)
                {
                    SoundEngine->play2D("audio/bleep.mp3", false);
                    box.Destroyed = true;
                    this->SpawnPowerUps(box);
                }
                else
                {
                    //Block is solid, shake effect
                    SoundEngine->play2D("audio/solid.wav", false);
                    ShakeTime = 0.05f;
                    Effects->Shake = true;
                }

                // collision resolution
                Direction dir = std::get<1>(collision);
                glm::vec2 diff_vector = std::get<2>(collision);

                //If Passthrough is inactive (or box is solid) perform normal bounce off box collisions.
                if (!(Ball->PassThrough && !box.IsSolid))
                {

                    if (dir == LEFT || dir == RIGHT) // horizontal collision
                    {
                        Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.x);
                        if (dir == LEFT)
                            Ball->Position.x += penetration; // move ball right
                        else
                            Ball->Position.x -= penetration; // move ball left;
                    }
                    else // vertical collision
                    {
                        Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                        // relocate
                        float penetration = Ball->Radius - std::abs(diff_vector.y);
                        if (dir == UP)
                            Ball->Position.y -= penetration; // move ball up
                        else
                            Ball->Position.y += penetration; // move ball down
                    }
                }
            }
        }
    }
    // check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*Ball, *Player);
    if (!Ball->Stuck && std::get<0>(result))
    {
        SoundEngine->play2D("audio/bleep.wav", false);

        // check where it hit the board, and change velocity based on where it hit the board
        float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
        float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
        float percentage = distance / (Player->Size.x / 2.0f);
        // then move accordingly
        float strength = 2.0f;
        glm::vec2 oldVelocity = Ball->Velocity;
        Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
        //Ball->Velocity.y = -Ball->Velocity.y;
        Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity); // keep speed consistent over both axes (multiply by length of old velocity, so total strength is not changed)
        // fix sticky paddle
        Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);

        //If sticky, stick the ball to the paddle
        Ball->Stuck = Ball->Sticky;
    }

    //Powerup collisions
    for (PowerUp& powerUp : this->PowerUps) {
        if (!powerUp.Destroyed) {

            //If the powerup isnt grabbed, destroy it
            if (powerUp.Position.y > this->Height)
                powerUp.Destroyed = true;

            if (CheckCollision(*Player, powerUp)) {
                //Collided with player, activate the powerup
                SoundEngine->play2D("audio/powerup.wav", false);
                ActivatePowerUp(powerUp);
                powerUp.Destroyed = true;
                powerUp.Activated = true;
            }
        }
    }
}

bool CheckCollision(GameObject& one, GameObject& two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
        two.Position.x + two.Size.x >= one.Position.x;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
        two.Position.y + two.Size.y >= one.Position.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two) // AABB - Circle collision
{
    // get center point circle first 
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),	// up
        glm::vec2(1.0f, 0.0f),	// right
        glm::vec2(0.0f, -1.0f),	// down
        glm::vec2(-1.0f, 0.0f)	// left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}

//Determines if Powerup should spawn
bool ShouldSpawn(unsigned int chance) {
    unsigned int random = rand() % chance;
    return random == 0;
}

//Spawn Powerups
void Game::SpawnPowerUps(GameObject& block) {

    //Speed Up
    if (ShouldSpawn(30)) //1 in 30 chance
    {
        this->PowerUps.push_back(
            PowerUp("speed-up", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("speed-up")));
    }

    //Speed Down
    if (ShouldSpawn(50)) //1 in 50 chance
    {
        this->PowerUps.push_back(
            PowerUp("speed-down", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("speed-down")));
    }

    //Sticky
    if (ShouldSpawn(60)) //1 in 60 chance
    {
        this->PowerUps.push_back(
            PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("sticky")));
    }

    //Pass-Through
    if (ShouldSpawn(75)) //1 in 75 chance
    {
        this->PowerUps.push_back(
            PowerUp("pass-through", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("pass-through")));
    }

    //pad-size-increase
    if (ShouldSpawn(50)) //1 in 50 chance
    {
        this->PowerUps.push_back(
            PowerUp("pad-increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position, ResourceManager::GetTexture("pad-increase")));
    }
    //pad-size-decrease
    if (ShouldSpawn(40)) //1 in 40 chance
    {
        this->PowerUps.push_back(
            PowerUp("pad-decrease", glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, block.Position, ResourceManager::GetTexture("pad-decrease")));
    }

    //Confuse
    if (ShouldSpawn(30)) //1 in 30 chance
    {
        this->PowerUps.push_back(
            PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("confuse")));
    }

    //Chaos
    if (ShouldSpawn(30)) //1 in 15 chance
    {
        this->PowerUps.push_back(
            PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("chaos")));
    }

    //Life Up
    if (ShouldSpawn(100)) //1 in 100 chance
    {
        this->PowerUps.push_back(
            PowerUp("life-up", glm::vec3(1.0f, 0.5f, 0.5f), 0.0f, block.Position, ResourceManager::GetTexture("life-up")));
    }
}

void ActivatePowerUp(PowerUp& powerUp) {

    //Speed Power Ups
    if (powerUp.Type == "speed-up")
    {
        Ball->Velocity *= 1.2f;
    }
    else if (powerUp.Type == "speed-down")
    {
        float halfInitialVelocity = INITIAL_BALL_VELOCITY.y / 2.0;

        if (Ball->Velocity.y > halfInitialVelocity)
            Ball->Velocity *= 0.8f;
        else
        {
            Ball->Velocity.x = halfInitialVelocity;
            Ball->Velocity.y = halfInitialVelocity;
        }
    }

    //Sticky
    else if (powerUp.Type == "sticky") {
        Ball->Sticky = true;
        Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
    }

    //Pass-Through
    else if (powerUp.Type == "pass-through") {
        Ball->PassThrough = true;
        Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
    }

    //Pad Size Power Ups
    else if (powerUp.Type == "pad-increase") {
        Player->Size.x += 50; //Pixels
    }
    else if (powerUp.Type == "pad-decrease") {

        float halfSize = PLAYER_SIZE.x / 2.0f;

        if (Player->Size.x > halfSize + 50)
            Player->Size.x -= 50; //Pixels
        else
            Player ->Size.x = halfSize;
    }

    //Confusion
    else if (powerUp.Type == "confuse") {
        if (!Effects->Chaos) {
            Effects->Confuse = true; //Only active if chaos is off
        }
    }

    //Chaos
    else if (powerUp.Type == "chaos") {
        if (!Effects->Confuse) {
            Effects->Chaos = true; //Only active if confusion is off
        }
    }

    //Life Up
    else if (powerUp.Type == "life-up") {
        Player->Lives++;
    }
}

void Game::UpdatePowerUps(float dt)
{
    for (PowerUp& powerUp : this->PowerUps)
    {
        powerUp.Position += powerUp.Velocity * dt;

        if (powerUp.Activated)
        {
            powerUp.Duration -= dt;

            if (powerUp.Duration <= 0.0f)
            {
                //Remove powerup from list (will later be removed)
                powerUp.Activated = false;

                //deactivate effects
                if (powerUp.Type == "sticky") {
                    if (!IsOtherPowerUpActive(this->PowerUps, "sticky")) {
                        //Only reset if no other powerup of type sticky is active
                        Ball->Sticky = false;
                        Player->Color = glm::vec3(1.0f);
                    }
                }
                else if (powerUp.Type == "pass-through")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "pass-through"))
                    {	// only reset if no other PowerUp of type pass-through is active
                        Ball->PassThrough = false;
                        Ball->Color = glm::vec3(1.0f, 0.0f, 0.0f);
                    }
                }
                else if (powerUp.Type == "confuse")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "confuse"))
                    {	// only reset if no other PowerUp of type confuse is active
                        Effects->Confuse = false;
                    }
                }
                else if (powerUp.Type == "chaos")
                {
                    if (!IsOtherPowerUpActive(this->PowerUps, "chaos"))
                    {	// only reset if no other PowerUp of type chaos is active
                        Effects->Chaos = false;
                    }
                }
            }
        }
    }
    this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(),
        [](const PowerUp& powerUp) { return powerUp.Destroyed && !powerUp.Activated; }
    ), this->PowerUps.end());
}

//Check if there is a powerup of the same type currently active, if so, we dont want to deactivate the effect.
bool IsOtherPowerUpActive(std::vector <PowerUp>& powerUps, std::string type){
    for (const PowerUp& powerUp : powerUps) {
        if (powerUp.Activated)
            if (powerUp.Type == type)
                return true; //Other powerup found, keep effect.
    }
    return false; //Safe to turn off effect
}