/*
world_tile Class (world_tile.h, world_tile.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that stores information able the a tile of the world including what game object is occupying it, and the graphical represetnation of it
*/

#ifndef __WORLDTILE_H
#define __WORLDTILE_H

#include "SFML\Graphics.hpp"
#include "pickup.h"
#include <ctime>

#define DEFAULT_TILE_COST 10
#define TILE_GENERATION_TIME 3
#define RESOURCE_VALUE 50

class GameObject;

class world_tile
{
private:
	//The grid position on the world grid
	int position[2]; 
	sf::Sprite sprite;
	sf::Sprite lastObjectOnIt;
	std::string type;
	//Id of the game object that is currently occupying this tile. if is negative it is not occupied. 
	int occupiedId;
	std::vector<GameObject*> occupiedObj;
	// Pathfinding cost
	int cost;
	//Pointer to a pickup if one is on the tile
	pickup* tilePickup;
	bool createsResources;
	time_t timeOfLastSpawn;
	sf::Texture* pickupTexture;
	float pickupSize;
public:
	
	world_tile();
	~world_tile();
	void setTextureToLast(sf::Texture* textureToSet, float squareSize, int xCord, int yCord);
	void copyASpriteToTheLastSeen(sf::Sprite copiedSprite);
	sf::Sprite* getSprite();
	int getOccupiedId();
	GameObject* getOccupiedObj();
	void setOccupiedObj(GameObject* objToSet);
	void setPosition(const int x, const int y);
	const int getX();
	const int getY();
	//Returns true if object successfully claims tile
	bool tryClaimTile(GameObject* objToClaim);
	void freeTile(GameObject* objectLeaving);
	const int getCost();
	void setCost(const int cost);
	pickup* claimPickupIfExists();
	void setResourceGeneration(const bool status);
	void tryCreateResource(sf::Texture* pickupTexture, float size);
	bool hasResource();
	sf::Sprite* getResourceSprite();
	void createResources(sf::Texture* pickupTexture, float size);
	pickup* checkPickup();
	bool checkIfThisObjInHere(GameObject* objToCheckFor);
	int numberOfObjectsOnMe();
	GameObject* getObjectOnMeWithIndex(int index);
};

#endif