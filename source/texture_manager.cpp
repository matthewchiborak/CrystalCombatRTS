#include "../include/texture_manager.h"



TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
	//Delete all the textures that were dynamically created in the vector of textures
	for (int i = 0; i < textures.size(); i++)
	{
		delete textures.at(i);
	}
}

TextureManager * TextureManager::getTextureManager()
{
	if (!s_instance)
	{
		s_instance = new TextureManager();
	}

	return s_instance;
}

bool TextureManager::deleteTexture(std::string name)
{
	//Search for the texture based on its name
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures.at(i)->getName() == name)
		{
			delete textures.at(i);
			textures.erase(textures.begin() + i);
			return true;
		}
	}

	//Couldn't find the texture.
	return false;
}

bool TextureManager::deleteTexture(int index)
{
	//Search for the texture based on its index. Much faster in terms of searching but might not know the index.
	if (index >= textures.size())
	{
		//Request was out of the range of the vector
		return false;
	}

	delete textures.at(index);
	textures.erase(textures.begin() + index);
	return true;
}

int TextureManager::getNumberOfTextures()
{
	return textures.size();
}

//Returns true if load was successful. False if not
bool TextureManager::loadTexture(std::string name, std::string location)
{
	//Check to make sure the texture doesn't already exist
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures.at(i)->getName() == name)
		{
			return false;
		}
	}

	GameTexture* newGameTexture = new GameTexture(name);

	if (newGameTexture->getTexture()->loadFromFile(location))
	{
		//Load was successful
		textures.push_back(newGameTexture);
		return true;
	}

	//Load was unsuccessful
	delete newGameTexture;
	return false;
}

// Get a stored texture by the texture's stored name
sf::Texture* TextureManager::getTexture(std::string name)
{
	//Search for the texture based on its name
	for (int i = 0; i < textures.size(); i++)
	{
		if (textures.at(i)->getName() == name)
		{
			return textures.at(i)->getTexture();
		}
	}

	//Couldn't find the texture. Return a null pointer
	return nullptr;
}

// Get a stored texture by its index in the vector
sf::Texture* TextureManager::getTexture(int index)
{
	//Search for the texture based on its index. Much faster in terms of searching but might not know the index.
	if (index >= textures.size())
	{
		//Request was out of the range of the vector
		return nullptr;
	}

	//Return the requested texture
	return textures.at(index)->getTexture();
}
