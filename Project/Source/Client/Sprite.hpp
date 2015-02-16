#pragma once

#include <string>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SOIL.h>
#include "Vector2.hpp"

class Sprite
{
	private:
		GLuint textureID;
		Vector2 position, dimensions;
		float angle;
		std::string filename;

	public:
		Sprite();
		Sprite(char* path, Vector2 position, Vector2 dimensions);
		~Sprite();

		void loadFromFile(const char* path);
		void render();

		void addAngle(float r);
		float getAngle();
		void setAngle(float a);

		void setPosition(Vector2 newPos);
		Vector2 getPosition();

		std::string getFilename();
};

