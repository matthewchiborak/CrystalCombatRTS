/*
TextureManager Class (texture_manager.h, texture_manager.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that stores all the textures that will be used by the game for sprites. 
*/

#ifndef __TEXTUREMANAGER_H
#define __TEXTUREMANAGER_H

#include "SFML\Graphics.hpp"
#include <iostream>
#include <string>
#include <vector>

#include "game_texture.h"

class TextureManager
{
	private:
		std::vector<GameTexture*> textures;
		static TextureManager* s_instance;
		TextureManager();

	public:
		
		~TextureManager();

		static TextureManager* getTextureManager();

		bool deleteTexture(std::string name);
		bool deleteTexture(int index);

		int getNumberOfTextures();

		// Get a stored texture by the texture's stored name
		sf::Texture* getTexture(std::string name);

		// Get a stored texture by its index in the vector
		sf::Texture* getTexture(int index);

		//Returns true if load was successful. False if not
		bool loadTexture(std::string name, std::string location); 

		
};

#endif