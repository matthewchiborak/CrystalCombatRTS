/*
GameTexture Class (game_texture.h, game_texture.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that contains the information of textures to be applied to the game sprites. Makes it easier to manage the textures. 
*/

#ifndef __GAMETEXTURE_H
#define __GAMETEXTURE_H

#include "SFML\Graphics.hpp"
#include <string>

class GameTexture
{
	private:
		std::string name;
		int size;
		sf::Texture texture;

	public:
		GameTexture(std::string name);
		~GameTexture();
		sf::Texture* getTexture();
		std::string getName();
		void setTexture(sf::Texture texture);
};

#endif
