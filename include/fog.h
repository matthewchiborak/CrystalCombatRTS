/*
	Fog Class (fog.h, fog.cpp)

	Created: Jan 20ish, 2017
	Author: Matthew Chiborak

	Description:
	Class that is used for both the graphical representation of the fog of war 
	to the player but also the logic of if either the player or the opponent 
	can see the space. Will be used for path finding and the gamescreen. 
*/

#ifndef __FOG_H
#define __FOG_H

#include <SFML\Graphics.hpp>


class Fog
{
private: 
	sf::Sprite fogSprite;
	sf::Sprite transparentFog;
	sf::Sprite lastSpriteSeen;
	sf::Sprite resetLastSeenSprite;
	bool isEnabled; //For the player. Also effects the visual effect. THIS IS THE TILE HAS NEVER HAD VISION ON BEFORE
	bool isEnabledOpponent; // If the tile is hidden from the opponent
	bool isCurrentlyVisiblePlayer;
	bool isCurrentlyVisibileOpponent;
	int idLastSawInPlayer;
	int idLastSawInOpponent;
	bool lastSeenCordOccupies; //Only need for player because is purely visual
public:
	static bool unitSeen(Fog** fog, bool isPlayer, const int id, const int xCord, const int yCord);
public:
	Fog();
	~Fog();
	void setTexture(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord);
	void setTransparency(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord);
	void setLastSeen(sf::Texture* textureToSet, const float squareSize, const int xCord, const int yCord);
	sf::Sprite& getSprite();
	sf::Sprite& getDarkness();
	sf::Sprite& getLastSpriteSeen();
	void resetLastSeen();
	void copyASpriteToTheLastSeen(sf::Sprite copiedSprite, const float squareSize, const int xCord, const int yCord);
	//Set and get for the player visibility
	void setEnabled(const bool status);
	const bool getEnabled();
	//For opponent
	void setEnabledOpponent(const bool status);
	const bool getEnabledOpponent();
	//Current Visible
	void setCurrentVisiblePlayer(const bool status);
	const bool getCurrentVisiblePlayer();
	void setCurrentVisibleOpponent(const bool status);
	const bool getCurrentVisibleOpponent();
	//Last Seen setters and getters
	const int getLastSeenIdPlayer();
	const int getLastSeenIdOpponent();
	void setLastSeenIdPlayer(const int id);
	void setLastSeenIdOpponent(const int id);
	
	bool getLastSeenCordOccupies();
	void setLastSeenCordOccupies(const bool status);
};

#endif