#include <iostream>
#include <sstream>

#include "Server.h"

#include <SDL.h>

Server::Server()
{
	// we want winsock version 2.2
	WORD sockVersion = MAKEWORD(2, 2);

	// load up winsock
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		std::cout << "Failed to start winsock library: " << WSAGetLastError() << std::endl;
		return;
	}

	// create the listening socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		std::cout << "Failed to create socket." << std::endl;
		return;
	}

	//Change socket to a non-blocking one
	u_long iMode = 1;
	if (ioctlsocket(s, FIONBIO, &iMode) == SOCKET_ERROR)
	{
		std::cout << "Socket did not enter non-blocking mode." << std::endl;
		return;
	}

	do
	{
		std::cout << "Please enter port number (1000-99999): ";
		std::cin >> port;
	} while (port < 1000 || port > 99999);
		
	SOCKADDR_IN serverInfo;
	serverInfo.sin_family = AF_INET;
	serverInfo.sin_addr.s_addr = INADDR_ANY;
	serverInfo.sin_port = port;

	// bind the socket
	if (bind(s, (struct sockaddr*)&serverInfo, sizeof(serverInfo)) == SOCKET_ERROR)
	{
		std::cout << "Failed to bind socket." << std::endl;
		return;
	}

	do
	{
		std::cout << "Please enter tickrate (16-128): ";
		std::cin >> tickrate;
	} while (tickrate < 16 || tickrate > 128);

	do
	{
		std::cout << "Please enter player count (2-4): ";
		std::cin >> maxPlayers;
	} while (maxPlayers < 2 || maxPlayers > 4);

	connectedPlayers = 0;
	readyPlayers = 0;

	do
	{
		std::cout << "Please enter round limit (best of 3-31): ";
		std::cin >> bestOfX;
	} while (bestOfX < 3 || bestOfX > 31);

	do
	{
		std::cout << "Please enter mapID (0-1): ";
		std::cin >> mapID;
	} while (mapID < 0 || mapID > 1);


	data = new char[BUFLEN];

	state = ServerState_Waiting;
	gameOver = false;
}

Server::~Server()
{
	if (!gameOver){
		if (!sendToAllClients("shutdown", strlen("shutdown")))
			std::cout << "Failed to send shutdown packet to a player." << std::endl;
	}

	for (int i = 0; i < players.size(); i++){
		delete players[i];
	}

	for (int i = 0; i < finishedPlayers.size(); i++){
		delete finishedPlayers[i];
	}
	
	if (closesocket(s) != 0)
		std::cout << "Error shutting down socket." << std::endl;

	WSACleanup();
}

int Server::getMaxPlayers()
{
	return maxPlayers;
}

int Server::getTickrate()
{
	return tickrate;
}

bool Server::sendToClient(PlayerInfo player, std::string data)
{
	char d[BUFLEN];
	strcpy(d, data.c_str());
	return sendToClient(player, d, strlen(d));
}

bool Server::sendToClient(PlayerInfo player, char* data, int size)
{
	return sendto(s, data, size, 0, (struct sockaddr*)&player.getAddress(), sizeof(player.getAddress())) != SOCKET_ERROR;
}

bool Server::sendToAllClients(char* data, int size)
{
	bool success = true;

	for (PlayerInfo *player : players)
	{
		if (!sendToClient(*player, data, size))
			success = false;
	}

	return success;
}

void Server::handleIncomingTraffic(std::string packet, sockaddr_in clientInfo)
{
	if (state == ServerState_Waiting)
	{
		if (packet.substr(0, 7).compare("welcome") == 0)
		// if we received a welcome packet from a player signing up with their name
		{
			bool skip = false;

			// check if that player has already been marked as signed up
			for (PlayerInfo *playerInfo : players)
			{
				if (*playerInfo == clientInfo)
				// this means we received another welcome packet from a client that is already registered
				{
					// answer with a gameinfo packet that also serves as an ack
					std::stringstream msg;
					msg << "gameinfo:" << maxPlayers << ":" << bestOfX << ":" << mapID;
					sendToClient(*playerInfo, msg.str());
					skip = true;
					break;
				}
			}

			if (!skip)
			// if we received a NEW player
			{
				std::string nickname = packet.substr(8);

				std::cout << "Player \"" << nickname.c_str() << "\" (" << (USHORT)clientInfo.sin_addr.S_un.S_un_b.s_b1 << "."
					<< (USHORT)clientInfo.sin_addr.S_un.S_un_b.s_b2 << "." << (USHORT)clientInfo.sin_addr.S_un.S_un_b.s_b3 << "."
					<< (USHORT)clientInfo.sin_addr.S_un.S_un_b.s_b4 << ":" << clientInfo.sin_port << ") connected (" << connectedPlayers + 1
					<< "/" << maxPlayers << ")" << std::endl;

				connectedPlayers++;

				players.push_back(new PlayerInfo(clientInfo, nickname));
			}
		}
		else if (packet.substr(0, 9).compare("start:ack") == 0)
		// if we received a start acknowledge packet from a player
		{
			for (PlayerInfo *playerInfo : players)
			// loop through all connected players
			{
				if (*playerInfo == clientInfo)
				// if we found the player that send us the ack packet
				{
					if (!playerInfo->hasAcknowledgedStartPacket)
					// if the player has not acknowledged their start packet yet
					{
						// mark the player as having acknowledged their start packet
						playerInfo->hasAcknowledgedStartPacket = true;

						//std::cout << "Player \"" << playerInfo->getName() << "\" is ready and has acknowledged their start packet." << std::endl;

						readyPlayers++;

						if (readyPlayers == maxPlayers)
						{
							std::cout << std::endl << "All players ready, starting game..." << std::endl << std::endl;
							state = ServerState_Ingame;
						}
					}
				}
			}
		}
		else
			std::cout << "Invalid packet received (\"" << data << "\") and discarded." << std::endl;
	}
	else if (state == ServerState_Ingame)
	{
		int index = 0;

		if (packet.substr(0, 8).compare("gameover") == 0){
		
			bool newFinishedPlayer = true;

			for (PlayerInfo* playerInfo : finishedPlayers){
				if (*playerInfo == clientInfo){
					newFinishedPlayer = false;
				}
			}

			if (newFinishedPlayer){
				finishedPlayers.push_back(new PlayerInfo(clientInfo, ""));
				sendToClient(*finishedPlayers.back(), "ack:gameover");
			}

			if (finishedPlayers.size() >= connectedPlayers){
				gameOver = true;
			}
		}

		for (PlayerInfo *playerInfo : players)
		{
			if (*playerInfo == clientInfo)
			{
				float *playerData = new float[10];
				memcpy(playerData, data, sizeof(float)* 10);

				playerInfo->positionX = playerData[0];
				playerInfo->positionY = playerData[1];
				playerInfo->forwardX = playerData[2];
				playerInfo->forwardY = playerData[3];
				playerInfo->angle = playerData[4];
				playerInfo->rocketPositionX = playerData[5];
				playerInfo->rocketPositionY = playerData[6];
				playerInfo->rocketForwardX = playerData[7];
				playerInfo->rocketForwardY = playerData[8];
				playerInfo->isDead = playerData[9];

				delete[] playerData;

				break;
			}

			index++;
		}
	}
}

void Server::handleOutgoingTraffic()
{
	if (state == ServerState_Waiting)
	// if we are still in waiting/lobby mode
	{
		if (connectedPlayers == maxPlayers)
		// and we are ready to go (all players have connected)
		{
			int playerID = 0;

			for (PlayerInfo *i : players)
			// loop through all connected players
			{
				if (!(*i).hasAcknowledgedStartPacket)
				// if this player has not ackowledged their start packet yet
				{
					// send them a new one

					std::stringstream msg;
					msg << "start:" << playerID << ":";
					
					for (PlayerInfo *tmp : players)
					// append a list of nicknames for all players to the start packet
					{
						if (i == tmp)
						// exclude the player himself
							continue;

						msg << tmp->getName() << ":";
					}

					if (!sendToClient(*i, msg.str()))
						std::cout << "Failed to send start packet to a player." << std::endl;
				}

				playerID++;
			}
		}
	}
	else if (state == ServerState_Ingame)
	{
		float playerData[BUFLEN];

		for (int i = 0; i < players.size(); ++i)
		{
			int offset = i * 10;

			playerData[offset + 0] = players[i]->positionX;
			playerData[offset + 1] = players[i]->positionY;
			playerData[offset + 2] = players[i]->forwardX;
			playerData[offset + 3] = players[i]->forwardY;
			playerData[offset + 4] = players[i]->angle;
			playerData[offset + 5] = players[i]->rocketPositionX;
			playerData[offset + 6] = players[i]->rocketPositionY;
			playerData[offset + 7] = players[i]->rocketForwardX;
			playerData[offset + 8] = players[i]->rocketForwardY;
			playerData[offset + 9] = players[i]->isDead;
		}

		char allPlayerData[BUFLEN];
		memcpy(allPlayerData, playerData, maxPlayers * sizeof(float)* 10);
		sendToAllClients(allPlayerData, maxPlayers * sizeof(float)* 10);
	}
}

bool Server::update()
{
	//clear the buffer
	memset(data, '\0', BUFLEN);

	// read data from socket
	sockaddr_in clientInfo;
	int clientLen = sizeof(clientInfo);
	int bytesReceived = recvfrom(s, data, BUFLEN, 0, (struct sockaddr*)&clientInfo, &clientLen);

	while (bytesReceived > 0)
	// if there was something to read
	{
		// handle read packet
		handleIncomingTraffic(data, clientInfo);

		// read more data
		bytesReceived = recvfrom(s, data, BUFLEN, 0, (struct sockaddr*)&clientInfo, &clientLen);
	}

	handleOutgoingTraffic();

	return gameOver;
}