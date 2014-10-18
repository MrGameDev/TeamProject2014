#pragma once

#include "Transform.hpp"
#include "Inputlistener.h"

class Player : public Transform, public Inputlistener
{
	private:

	public:
		Player(Vector2 position, Vector2 forward);

		virtual void inputReceived(SDL_KeyboardEvent *key);

		void render();
};