#include "../include/pickup.h"



pickup::pickup(sf::Texture* pickupTexture, const float size, const int value)
{
	this->value = value;

	pickupSprite.setTexture(*pickupTexture);


	pickupSprite.setScale(sf::Vector2f((size) / pickupTexture->getSize().x, (size) / (float)pickupTexture->getSize().y));
}


pickup::~pickup()
{
}

sf::Sprite* pickup::getSprite()
{
	return &pickupSprite;
}

void pickup::addToValue(int addition)
{
	value += addition;
}

int pickup::getValue()
{
	return value;
}
