/*
Renderer Class (Renderer.h, Renderer.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that stores and draws the game objects.
*/

#ifndef __RENDERER_H
#define __RENDERER_H

#include "SFML\Graphics.hpp"
#include "game_object.h"
#include "world_tile.h"
#include "fog.h"
#include "logger.h"
#include "hud.h"
#include <string>
#include <vector>
#include "pickup.h"


#define MENU_SIZE_X 10
#define MENU_SIZE_Y 2
#define HEALTH_BAR_OFFSET 10
#define DEFAULT_VISION_RANGE 3
#define HEALTH_BAR_SCALE 0.25

class Game;

static const bool attackTargetInRange(GameObject *attacker, GameObject *target)
{
	// Validate inputs
	if (!attacker || !target)
		return false;

	// In X-range
	if (// On left side
		(attacker->getGridPosX() <= target->getGridPosX() &&
			attacker->getGridPosX() + attacker->getAttackRange() >= target->getGridPosX()) ||
		// On right side
		(attacker->getGridPosX() > target->getGridPosX() &&
			attacker->getGridPosX() - attacker->getAttackRange() <= target->getGridPosX() + target->getGridSizeX() - 1))
	{
		// In Y-range
		if (// Above target
			(attacker->getGridPosY() <= target->getGridPosY() &&
				attacker->getGridPosY() + attacker->getAttackRange() >= target->getGridPosY()) ||
			// Below target
			(attacker->getGridPosY() > target->getGridPosY() &&
				attacker->getGridPosY() - attacker->getAttackRange() <= target->getGridPosY() + target->getGridSizeY() - 1))
		{
			return true;
		}
	}

	// Not in range
	return false;
}

class Renderer
{
private:

	std::vector<GameObject*> playerInGameObjects;
	std::vector<GameObject*> opponentInGameObjects;

	std::vector<GameObject*> structureRefsPlayer;
	std::vector<GameObject*> structureRefsOpponent;

	//Refs to the source tiles to tell tehm to make resources
	std::vector<world_tile*> resourceTiles;

	sf::RenderWindow* gameWindow;
	Game *game;
	sf::View mainView;
	sf::View HUD;
	int windowSizeX;
	int windowSizeY;
	int windowPosX;
	int windowPosY;
	float windowScrollOffset[2];
	float squareSize;
	float zoom;
	float minZoom;
	float maxZoom;
	world_tile** backgroundTiles;
	Fog** fogOfWar;
	int worldSizeX;
	int worldSizeY;

	//For detecting mouse clicks
	float bValueX;
	float cValueX;
	float bValueY;
	float cValueY;

	//For the selection outline

	//?????? This causing the stack overflow??????
	sf::Texture* selectionTexture;

	//For fog of war masking
	sf::Sprite largeFogOfWar;
	sf::Sprite largeFog;
	sf::Texture* largeForOfWarTexture;
	sf::Texture visionMaskTexture;
	sf::Texture fogTex;
	float fogScaleX;
	float fogScaleY;

	//For HUD
	hud playerInterface;
	sf::Font* menuFont;
	/*sf::Sprite HUDSprite;
	sf::Sprite menuSelector;
	float menuHeight;*/

	std::mutex gameObjectLock;
	sf::Texture* pickupTexture;

	sf::Sprite backgroundImage;

	//Game over condition base. If either of these are killed then the game is over
	GameObject* playerBase;
	GameObject* enemyBase;


public:
	Renderer(Game * game, sf::RenderWindow* passedWindow, int windowSizeX, int windowSizeY, int windowPosX, int windowPosY, float squareSize, float minZoom, float maxZoom, int worldSizeX, int worldSizeY, sf::Font* menuFont);
	~Renderer();
	GameObject * addObjectToScene(const bool isPlayer, const bool isStructure, const bool beingBuilt, const std::string type, const int xPos, const int yPos, const int sizeX, const int sizeY, sf::Texture* textureToSet, const float health, const float speed, const int visionRange, const int attackRange, const float attackPower, const float attackSpeed, const bool flying, const std::string createsType);
	//bool addObjectToScene(bool isPlayer, bool isStructure, int xPos, int yPos, int sizeX, int sizeY, sf::Texture* textureToSet, float health, float speed, int visionRange);
	void addObjectToScene(const bool isPlayer, GameObject* gameObjectToAdd);
	void drawObjectWithView(Game *game);
	GameObject* getGameObject(bool isPlayer, int index);
	bool removeObjectFromScene(GameObject* gameObjectToRemove); //Returns true if removal was successful. False if otherwise
	bool removeObjectFromScene(int id); //Remove the gameobject from the scene that matches the passed ID number stored within the game object.
	void zoomView(float zoomIncrement);
	void setZoomView(float zoomValue);

	//void drawScene(float x, float y, int playerResourceCount);

	void drawScene(Game *game);

	void drawBackgroundTiles();
	void drawFogOfWar();
	void giveFogTilesATexture(int xCord, int yCord, sf::Texture* textToSet, float squareSize);
	void giveBigFogATexture(sf::Texture* textToSet, float squareSize);
	void giveFogTilesATransparency(int xCord, int yCord, sf::Texture* textToSet, float squareSize);
	void giveFogTilesABlankLastSeen(int xCord, int yCord, sf::Texture* textToSet, float squareSize);
	void setBackgroundTilePosition(const int xCord, const int yCord);
	void tellObjectsToDoActions(Game* game);
	void giveHUDtheUnitCostInfo(std::vector<std::string>* unitCostInfo);

	world_tile* getTile(const int xCord, const int yCord);
	world_tile* getTileWithMouseCords(bool* foundTheTile, float x, float y);
	world_tile* getNearestTileWithMouseCords(bool* foundTheTile, float x, float y);

	const int getWorldSizeX();
	const int getWorldSizeY();

	//Returns true if an object was found. The x and y are the mouse cordinates in the window
	//bool getGameObjectAtThisLocation(bool isPlayerObject, GameObject* wantedObject, float x, float y);
	GameObject* getGameObjectAtThisLocation(bool isPlayerObject, bool* foundTheObj, float x, float y);

	//Move the camera window position. Will be used when move the mouse to the edge of the screen
	void moveCamera(float xMove, float yMove);

	//Seting the selection texture to show that a mouse is hovering over an object
	void setSelectionTexture(Game* game, sf::Texture* textureToSet, int* currentlySelectedMenu);

	//For fogofwar masking
	void setFogOfWarTextureForMasking(sf::Texture* textureToSet);
	void setVisionMaskTexture(sf::Texture* textureToSet);

	void setBackgroundTexture(sf::Texture* textureToSet, Game* game);

	//For drawing the HUD
	void drawHUD(Game* game);
	//return the height of the menu to pass to the interface controller
	float giveHUDSpriteATexture(Game* game, sf::Texture* textToSet, float squareSize, int index, int totalIcons, float infoHeight);
	void setInfoBarSprite(sf::Texture* textureToSet, float squareSize, float infoBarHeight);

	float getMenuHeight();

	//Resizing the window logic. Returns the new menuheight
	float resizeWindow(float x, float y, Game *game);

	//Returns null if the game object is not owned by the desired player
	GameObject* getGameObjectWithId(bool isPlayer, int objectId);

	void resetVision();

	void setPickupTexture(sf::Texture* textToSet);

	//Checks if a unit carrying resources is close enough to a structure. If it is, takes the resources and adds it to its total. Returns the overal total to get added to the resource count.
	void giveBasesUnitResources(int* playerResources, int* opponentResources);

	//Tells all the resources tiles in the game world to try to create a resource
	void tellResourceTilesToTryToMakeResources();
	void addTileToListOfResourceTiles(int x, int y);

	void centerCamera(int gridX, int gridY);

	void changeZoom(int newSquareSize);

	void tellResourceTilesToMakeResources();

	//Function to be used by the agent to a viable base
	GameObject* getAgentBase(std::string type);

	GameObject* getGameObjectAgent(bool isPlayer, bool isStructure, std::string type, bool isBusy, bool isVisible);

	void setMainBase(bool isPlayer, GameObject* mainBase);

	void checkGameOver();

	void giveTitleScreen(sf::Texture* textToSet);
};

#endif