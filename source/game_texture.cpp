#include "../include/game_texture.h"



GameTexture::GameTexture(std::string name)
{
	this->name = name;
}


GameTexture::~GameTexture()
{
}

void GameTexture::setTexture(sf::Texture texture)
{
	this->texture = texture;
}

std::string GameTexture::getName()
{
	return name;
}

sf::Texture* GameTexture::getTexture()
{
	return &texture;
}
