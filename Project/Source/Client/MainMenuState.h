#pragma once

#include "Gamestate.h"
#include "Game.hpp"
#include <sstream>
#include <iostream>
#include <string>

enum PromptingState { PROMPTING_NAME, PROMPTING_IP, PROMPTING_PORT, PROMPTING_CL_PORT };

class MainMenuState : public Gamestate
{
	private:
		std::string name, ip, port, clPort;

		PromptingState prompt;
		std::stringstream stream;

	public:
		void init();
		void update();
		void render();
		void quit();
		void inputReceived(SDL_KeyboardEvent *key);
		void receivePacket(char* data);
		void appendSDLKey(SDL_KeyboardEvent *key, std::string *str);
};