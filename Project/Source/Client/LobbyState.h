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
	virtual void receivePacket(char* packet);
	virtual void update();
	virtual void render();
	virtual void quit();

	virtual void inputReceived(SDL_KeyboardEvent *key);

private:
	Client* client;

	bool isWelcomePacketAcknowledged;

	void sendWelcomePacketToServer();

};