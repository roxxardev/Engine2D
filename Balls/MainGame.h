#pragma once

#include <Engine/Camera2D.h>
#include <Engine/SpriteBatch.h>
#include <Engine/InputManager.h>
#include <Engine/Window.h>
#include <Engine/GLSLProgram.h>
#include <Engine/Timing.h>
#include <Engine/SpriteFont.h>
#include <memory>

#include "BallController.h"
#include "BallRenderer.h"
#include "Grid.h"


enum class GameState { RUNNING, EXIT };

const int CELL_SIZE = 12;

class MainGame {
public:
	~MainGame();
	void run();


private:
	void init();
	void initRenderers();
	void initBalls();
	void update(float deltaTime);
	void draw();
	void drawHud();
	void processInput();

	int m_screenWidth = 0;
	int m_screenHeight = 0;

	std::vector<Ball> m_balls; ///< All the balls
	std::unique_ptr<Grid> m_grid; ///< Grid for spatial partitioning for collision

	int m_currentRenderer = 0;
	std::vector<BallRenderer*> m_ballRenderers;

	BallController m_ballController; ///< Controls balls

	Engine::Window m_window; ///< The main window
	Engine::SpriteBatch m_spriteBatch; ///< Renders all the balls
	std::unique_ptr<Engine::SpriteFont> m_spriteFont; ///< For font rendering
	Engine::Camera2D m_camera; ///< Renders the scene
	Engine::InputManager m_inputManager; ///< Handles input
	Engine::GLSLProgram m_textureProgram; ///< Shader for textures]

	Engine::FpsLimiter m_fpsLimiter; ///< Limits and calculates fps
	float m_fps = 0.0f;

	GameState m_gameState = GameState::RUNNING; ///< The state of the game
};

