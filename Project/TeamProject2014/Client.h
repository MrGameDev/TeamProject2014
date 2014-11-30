#pragma once

#include <winsock.h>

// check out http://johnnie.jerrata.com/winsocktutorial/

class Client
{
	private:
		SOCKET serverSocket;
		char package[256];

	public:
		Client();
		~Client();

		void update();

		void setPackage(char* data, int size);
};