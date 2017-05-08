/*
hud Class (hud.h, hud.cpp)

Created: Jan 29, 2017
Author: Matthew Chiborak

Description:
Class the stores information related to the HUD. This includes the sprites for rendering the menu and the cost of a unit and the display of how much resources a unit has.
Interface controller determines if a menu option has been clicked and if the user has enough funds. 
*/


#ifndef __HUD_H
#define __HUD_H

#include "SFML\Graphics.hpp"
#include "game_object.h"
#include <sstream>
#include <iomanip>

class Game;
class world_tile;
class fog;

// Convert time data to string
static std::string timeToString(const int maxTime, const int timeStarted)
{
	// Seconds are one digit - pad the 0
	if(((maxTime - (time(0) - timeStarted)) % 60) < 10)
		return std::to_string((maxTime - (time(0) - timeStarted)) / 60) + ":0" + std::to_string((maxTime - (time(0) - timeStarted)) % 60);
	else
		return std::to_string((maxTime - (time(0) - timeStarted)) / 60) + ":" + std::to_string((maxTime - (time(0) - timeStarted)) % 60);
}

// Convert info data to precise string
static std::string floatToString(const float value, const std::streamsize precision = 6)
{
	std::stringstream stream;
	
	// Is conversion necessary?
	if ((int)value < value)
		stream << std::fixed << std::setprecision(precision) << value;
	else
		stream << (int)value;
	return stream.str();
}

class hud
{
private:
	sf::Sprite* HUDSprite;
	sf::Sprite menuSelector;
	sf::Sprite menuSelector2;
	float menuHeight;
	sf::Text resourceCount;
	sf::Font* menuFont;
	std::vector<std::string>* unitCostInfo;
	sf::Text costText;
	sf::Sprite infoBarSprite;

	sf::Text unitInfoDisplay;
	int* currentlyDisplayedMenu;
	int totalIcons;

	sf::Sprite minimap;

	sf::Text timeText;
	sf::Text killFeedText;

	sf::Text pausedText;

	sf::Sprite titleScreen;
	sf::Text titleScreenText;

	/*sf::Sprite infoBarIcon;*/
	//sf::Texture tempTexture;

	//Minimap information
	//sf::Vector2f miniMapSize;
	//sf::Vector2f miniMapPosition;
	

public:
	hud();
	~hud();
	void setSelectionTexture(Game* game, sf::Texture* textureToSet, int windowSizeX, float squareSize, int menuSizeX, int menuSizeY, int* currentlySelectedMenu);
	//void drawHUD(sf::RenderWindow* gameWindow, int windowSizeX, int windowSizeY, float squareSize, int menuSizeX, int menuSizeY, float x, float y, int playerResourceCount, int currentMenuSelected, GameObject* selectedObject, float infoHeight, sf::Vector2i mouseStartPos, sf::Vector2i mousePos);
	void drawHUD(Game *game, sf::RenderWindow* gameWindow, int windowSizeX, int windowSizeY, int menuSizeX, int menuSizeY, world_tile** backgroundTiles, Fog** fogOfWar, std::vector<GameObject*>* playerInGameObjects, std::vector<GameObject*>* opponentInGameObjects, int windowPosX, int windowPosY, float zoom, float bx, float cx, float by, float cy);
	float giveHUDSpriteATexture(Game* game, sf::Texture* textToSet, float squareSize, int windowSizeX, int windowSizeY, int menuSizeX, int menuSizeY, int index, int totalIcons, float infoHeight);
	void assignFont(sf::Font* menuFont);
	void setUnitCostInfo(std::vector<std::string>* unitCostInfo);
	float getMenuHeight();
	void setInfoBarSprite(sf::Texture* textureToSet, int windowSizeX, int windowSizeY, float squareSize, float infoBarHeight);
	//returns the new menu height
	void hud::setMinimapTexture(Game* game, sf::Texture* textToSet);
	void hud::resizeMinimap(Game* game);
	void giveTItleScreenATexture(Game* game, sf::Texture* textToSet);
	float resizeHUD(Game *game, int windowSizeX, int windowSizeY, float squareSize, int menuSizeX, int menuSizeY, float x, float y, int playerResourceCount, int currentMenuSelected, GameObject* selectedObject, float infoHeight);
};

#endif