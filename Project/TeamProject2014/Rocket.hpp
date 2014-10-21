#pragma once

#include "Transform.hpp"
#include "InputObserver.h"
#include "Timer.h"

#include <SDL_opengl.h>

class Rocket : public Transform, public Inputlistener
{
	private:
		static const float TURN_ACCELERATION;
		static const float TURN_SPEED;

		bool controllable, leftKeyDown, rightKeyDown;

	public:
		Rocket(Vector2 position, Vector2 forward);
		~Rocket();

		virtual void inputReceived(SDL_KeyboardEvent *key);

		void update();
		void render();
};

