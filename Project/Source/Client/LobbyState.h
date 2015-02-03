#pragma once

#include "Gamestate.h"
#include "Client.h"
#include "Game.hpp"
#include "FontRenderer.h"

class Client;

class LobbyState : public Gamestate{
public:
	LobbyState();
	~LobbyState();

	virtual void init();
	virtual void receivePacket(std::string packet);
	virtual void update();
	virtual void render();
	virtual void quit();

	virtual void inputReceived(SDL_KeyboardEvent *key);

private:
	Client* client;
	int numberOfPlayers;

	char* receivedBuffer;

};