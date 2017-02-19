#pragma once

namespace Engine {

class FpsLimiter {
public:
	FpsLimiter();
	void init(float maxFPS);

	void setMaxFPS(float maxFPS);

	void begin();

	float end(); //return the current FPS
private:
	void calculateFPS();

	float _fps;
	float _maxFPS;
	float _frameTime;
	unsigned int _startTicks;
};

}