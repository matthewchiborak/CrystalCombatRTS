#include "../include/Fog.h"



bool Fog::unitSeen(Fog** fog, bool isPlayer, const int id, const int xCord, const int yCord)
{
	if(!fog && !fog[xCord])
		return false;

	if (!isPlayer)
	{
		if (fog[xCord][yCord].getCurrentVisiblePlayer())
			return true;
		else if (fog[xCord][yCord].getLastSeenIdPlayer() == id)
			return true;
	}
	else
	{
		if (fog[xCord][yCord].getCurrentVisibleOpponent())
			return true;
		else if (fog[xCord][yCord].getLastSeenIdOpponent() == id)
			return true;
	}
	return false;
}

Fog::Fog()
{
	isEnabled = true;
	isEnabledOpponent = true;

	isCurrentlyVisiblePlayer = false;
	isCurrentlyVisibileOpponent = false;

	idLastSawInOpponent = -1;
	idLastSawInPlayer = -1;

	lastSeenCordOccupies = false;
}


Fog::~Fog()
{
}

void Fog::setTexture(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord)
{
	fogSprite.setTexture(*textureToSet);
	fogSprite.setOrigin(textureToSet->getSize().x / 2.0, textureToSet->getSize().y / 2.0);

	fogSprite.setPosition(sf::Vector2f(xCord * (squareSize)+(squareSize / 2.0), yCord * (squareSize)+(squareSize / 2.0)));
	fogSprite.setScale(sf::Vector2f((float)1 * (squareSize) / textureToSet->getSize().x, (float)1 * (squareSize) / (float)textureToSet->getSize().y));

}

void Fog::setTransparency(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord)
{
	transparentFog.setTexture(*textureToSet);
	transparentFog.setOrigin(textureToSet->getSize().x / 2.0, textureToSet->getSize().y / 2.0);

	transparentFog.setPosition(sf::Vector2f(xCord * (squareSize)+(squareSize / 2.0), yCord * (squareSize)+(squareSize / 2.0)));
	transparentFog.setScale(sf::Vector2f((float)1 * (squareSize) / textureToSet->getSize().x, (float)1 * (squareSize) / (float)textureToSet->getSize().y));

	
}



sf::Sprite &Fog::getSprite()
{
	return fogSprite;
}

sf::Sprite& Fog::getDarkness()
{
	return transparentFog;
}

sf::Sprite& Fog::getLastSpriteSeen()
{
	return lastSpriteSeen;
}

void Fog::resetLastSeen()
{
	lastSpriteSeen = resetLastSeenSprite;
}

void Fog::setLastSeen(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord)
{
	lastSpriteSeen.setTexture(*textureToSet);
	lastSpriteSeen.setOrigin(textureToSet->getSize().x / 2.0, textureToSet->getSize().y / 2.0);

	lastSpriteSeen.setPosition(sf::Vector2f(xCord * (squareSize)+(squareSize / 2.0), yCord * (squareSize)+(squareSize / 2.0)));
	lastSpriteSeen.setScale(sf::Vector2f((float)1 * (squareSize) / textureToSet->getSize().x, (float)1 * (squareSize) / (float)textureToSet->getSize().y));

	resetLastSeenSprite = lastSpriteSeen;
}

void Fog::copyASpriteToTheLastSeen(sf::Sprite copiedSprite, const float squareSize, const int xCord, const int yCord)
{
	lastSpriteSeen = copiedSprite;
	lastSpriteSeen.setOrigin(0, 0);
	lastSpriteSeen.setPosition(squareSize*xCord, squareSize*yCord);
}

void Fog::setEnabled(const bool status)
{
	isEnabled = status;
}

const bool Fog::getEnabled()
{
	return isEnabled;
}

void Fog::setEnabledOpponent(const bool status)
{
	isEnabledOpponent = status;
}
const bool Fog::getEnabledOpponent()
{
	return isEnabledOpponent;
}

void Fog::setCurrentVisiblePlayer(const bool status)
{
	isCurrentlyVisiblePlayer = status;
}
const bool Fog::getCurrentVisiblePlayer()
{
	return isCurrentlyVisiblePlayer;
}
void Fog::setCurrentVisibleOpponent(const bool status)
{
	isCurrentlyVisibileOpponent = status;
}
const bool Fog::getCurrentVisibleOpponent()
{
	return isCurrentlyVisibileOpponent;
}
const int Fog::getLastSeenIdPlayer()
{
	return idLastSawInPlayer;
}
const int Fog::getLastSeenIdOpponent()
{
	return idLastSawInOpponent;
}
void Fog::setLastSeenIdPlayer(const int id)
{
	idLastSawInPlayer = id;
}
void Fog::setLastSeenIdOpponent(const int id)
{
	idLastSawInOpponent = id;
}

bool Fog::getLastSeenCordOccupies()
{
	return lastSeenCordOccupies;
}
void Fog::setLastSeenCordOccupies(const bool status)
{
	lastSeenCordOccupies = status;
}




