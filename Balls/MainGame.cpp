#define _CRT_SECURE_NO_WARNINGS // To shut up the compiler about sprintf
#include "MainGame.h"

#include <Engine/Engine.h>
#include <Engine/ResourceManager.h>
#include <SDL/SDL.h>
#include <random>
#include <ctime>
#include <algorithm>
#include <cmath>

const float DESIRED_FPS = 60.0f;
const int MAX_PHYSICS_STEPS = 6;
const float MS_PER_SECOND = 1000; 
const float DESIRED_FRAMETIME = MS_PER_SECOND / DESIRED_FPS;
const float MAX_DELTA_TIME = 1.0f;

MainGame::~MainGame() {
	for (int i = 0; i < m_ballRenderers.size(); i++) {
		delete m_ballRenderers[i];
	}
}

void MainGame::run() {
	init();
	initBalls();

	Uint32 previousTicks = SDL_GetTicks();

	while (m_gameState == GameState::RUNNING) {
		m_fpsLimiter.begin();
		processInput();

		Uint32 newTicks = SDL_GetTicks();
		Uint32 frameTime = newTicks - previousTicks;
		previousTicks = newTicks; 
		float totalDeltaTime = (float)frameTime / DESIRED_FRAMETIME;

		int i = 0;

		while (totalDeltaTime > 0.0f && i < MAX_PHYSICS_STEPS) {
			float deltaTime = std::min(totalDeltaTime, MAX_DELTA_TIME);

			update(deltaTime);

			totalDeltaTime -= deltaTime;
			i++;
		}

		m_camera.update();
		draw();
		m_fps = m_fpsLimiter.end();
	}
}

void MainGame::init() {
	Engine::init();

	m_screenWidth = 1920;
	m_screenHeight = 1080;

	m_window.create("Ball Game", m_screenWidth, m_screenHeight, Engine::FULLSCREEN);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	m_camera.init(m_screenWidth, m_screenHeight);
	m_camera.setPosition(glm::vec2(m_screenWidth / 2.0f, m_screenHeight / 2.0f));

	m_spriteBatch.init();
	m_spriteFont = std::make_unique<Engine::SpriteFont>("Fonts/chintzy.ttf", 40);

	m_textureProgram.compileShaders("Shaders/textureShading.vert", "Shaders/textureShading.frag");
	m_textureProgram.addAttribute("vertexPosition");
	m_textureProgram.addAttribute("vertexColor");
	m_textureProgram.addAttribute("vertexUV");
	m_textureProgram.linkShaders();

	m_fpsLimiter.setMaxFPS(600.0f);

	initRenderers();

}

void MainGame::initRenderers() {
	m_ballRenderers.push_back(new BallRenderer);
	m_ballRenderers.push_back(new MomentumBallRenderer);
	m_ballRenderers.push_back(new VelocityBallRenderer(m_screenWidth, m_screenHeight));
	m_ballRenderers.push_back(new TrippyBallRenderer(m_screenWidth, m_screenHeight));
}

struct BallSpawn {
	BallSpawn(const Engine::ColorRGBA8& colr,
	float rad, float m, float minSpeed,
	float maxSpeed, float prob) :
	color(colr),
	radius(rad),
	mass(m),
	randSpeed(minSpeed, maxSpeed),
	probability(prob) {
	}
	Engine::ColorRGBA8 color;
	float radius;
	float mass;
	float probability;
	std::uniform_real_distribution<float> randSpeed;
};
#include <iostream>
void MainGame::initBalls() {

	m_grid = std::make_unique<Grid>(m_screenWidth, m_screenHeight, CELL_SIZE);

#define ADD_BALL(p, ...) \
	totalProbability += p; \
	possibleBalls.emplace_back(__VA_ARGS__);

	const int NUM_BALLS = 10000;

	std::mt19937 randomEngine((unsigned int)time(nullptr));
	std::uniform_real_distribution<float> randX(0.0f, (float)m_screenWidth);
	std::uniform_real_distribution<float> randY(0.0f, (float)m_screenHeight);
	std::uniform_real_distribution<float> randDir(-1.0f, 1.0f);

	std::vector <BallSpawn> possibleBalls;
	float totalProbability = 0.0f;

	int maxRadius = 10.0f;
	std::uniform_real_distribution<float> r1(2.0f, maxRadius <= CELL_SIZE/2.0f ? maxRadius : CELL_SIZE/2.0f);
	std::uniform_int_distribution<int> r2(0, 255);

	//ADD_BALL(1.0f, Engine::ColorRGBA8(255, 255, 255, 255),
	//	2.0f, 1.0f, 0.1f, 7.0f, totalProbability);
	//ADD_BALL(1.0f, Engine::ColorRGBA8(1, 254, 145, 255),
	//	2.0f, 2.0f, 0.1f, 3.0f, totalProbability);
	//ADD_BALL(1.0f, Engine::ColorRGBA8(177, 0, 254, 255),
	//	3.0f, 4.0f, 0.0f, 0.0f, totalProbability)
	//	ADD_BALL(1.0f, Engine::ColorRGBA8(254, 0, 0, 255),
	//	3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
	//ADD_BALL(1.0f, Engine::ColorRGBA8(0, 255, 255, 255),
	//	3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
	//ADD_BALL(1.0f, Engine::ColorRGBA8(255, 255, 0, 255),
	//	3.0f, 4.0f, 0.0f, 0.0f, totalProbability);
	for (int i = 0; i < 1000; i++) {
		ADD_BALL(1.0f, Engine::ColorRGBA8(r2(randomEngine), r2(randomEngine), r2(randomEngine), 255),
			r1(randomEngine), r1(randomEngine), 0.0f, 0.0f, totalProbability);
	}

	std::uniform_real_distribution<float> spawn(0.0f, totalProbability);

	m_balls.reserve(NUM_BALLS);

	BallSpawn* ballToSpawn = &possibleBalls[0];
	for (int i = 0; i < NUM_BALLS; i++) {
		float spawnVal = spawn(randomEngine);
		for (size_t j = 0; j < possibleBalls.size(); j++) {
			if (spawnVal <= possibleBalls[j].probability) {
				ballToSpawn = &possibleBalls[j];
				break;
			}
		}

		glm::vec2 pos(randX(randomEngine), randY(randomEngine));

		glm::vec2 direction(randDir(randomEngine), randDir(randomEngine));
		if (direction.x != 0.0f || direction.y != 0.0f) { // The chances of direction == 0 are astronomically low
			direction = glm::normalize(direction);
		}
		else {
			direction = glm::vec2(1.0f, 0.0f); // default direction
		}

		m_balls.emplace_back(ballToSpawn->radius, ballToSpawn->mass, pos, direction * ballToSpawn->randSpeed(randomEngine),
			Engine::ResourceManager::getTexture("Textures/circle.png").id,
			ballToSpawn->color);
		m_grid->addBall(&m_balls.back());
	}
}

void MainGame::update(float deltaTime) {
	m_ballController.updateBalls(m_balls, m_grid.get(), deltaTime, m_screenWidth, m_screenHeight);
}

void MainGame::draw() {
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);

	glm::mat4 projectionMatrix = m_camera.getCameraMatrix();

	m_ballRenderers[m_currentRenderer]->renderBalls(m_spriteBatch, m_balls, projectionMatrix);

	m_textureProgram.use();

	GLint textureUniform = m_textureProgram.getUniformLocation("mySampler");
	glUniform1i(textureUniform, 0);

	GLint pUniform = m_textureProgram.getUniformLocation("P");
	glUniformMatrix4fv(pUniform, 1, GL_FALSE, &projectionMatrix[0][0]);

	drawHud();

	m_textureProgram.unuse();

	m_window.swapBuffer();
}

void MainGame::drawHud() {
	const Engine::ColorRGBA8 fontColor(255, 0, 0, 255);
	char buffer[64];
	sprintf(buffer, "%.1f", m_fps);

	m_spriteBatch.begin();
	m_spriteFont->draw(m_spriteBatch, buffer, glm::vec2(0.0f, m_screenHeight - 32.0f),
		glm::vec2(1.0f), 0.0f, fontColor);
	m_spriteBatch.end();
	m_spriteBatch.renderBatch();
}

void MainGame::processInput() {
	m_inputManager.update();

	SDL_Event evnt;
	while (SDL_PollEvent(&evnt)) {
		switch (evnt.type) {
		case SDL_QUIT:
			m_gameState = GameState::EXIT;
			break;
		case SDL_MOUSEMOTION:
			m_ballController.onMouseMove(m_balls, (float)evnt.motion.x, (float)m_screenHeight - (float)evnt.motion.y);
			m_inputManager.setMouseCoords((float)evnt.motion.x, (float)evnt.motion.y);
			break;
		case SDL_KEYDOWN:
			m_inputManager.pressKey(evnt.key.keysym.sym);
			break;
		case SDL_KEYUP:
			m_inputManager.releaseKey(evnt.key.keysym.sym);
			break;
		case SDL_MOUSEBUTTONDOWN:
			m_ballController.onMouseDown(m_balls, (float)evnt.button.x, (float)m_screenHeight - (float)evnt.button.y);
			m_inputManager.pressKey(evnt.button.button);
			break;
		case SDL_MOUSEBUTTONUP:
			m_ballController.onMouseUp(m_balls);
			m_inputManager.releaseKey(evnt.button.button);
			break;
		}
	}

	if (m_inputManager.isKeyPressed(SDLK_ESCAPE)) {
		m_gameState = GameState::EXIT;
	}
	if (m_inputManager.isKeyPressed(SDLK_LEFT)) {
		m_ballController.setGravityDirection(GravityDirection::LEFT);
	}
	else if (m_inputManager.isKeyPressed(SDLK_RIGHT)) {
		m_ballController.setGravityDirection(GravityDirection::RIGHT);
	}
	else if (m_inputManager.isKeyPressed(SDLK_UP)) {
		m_ballController.setGravityDirection(GravityDirection::UP);
	}
	else if (m_inputManager.isKeyPressed(SDLK_DOWN)) {
		m_ballController.setGravityDirection(GravityDirection::DOWN);
	}
	else if (m_inputManager.isKeyPressed(SDLK_SPACE)) {
		m_ballController.setGravityDirection(GravityDirection::NONE);
	}

	if (m_inputManager.isKeyPressed(SDLK_1)) {
		m_currentRenderer++;
		if (m_currentRenderer >= m_ballRenderers.size()) {
			m_currentRenderer = 0;
		}
	}
}