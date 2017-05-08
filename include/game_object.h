/*
GameObject Class (game_object.h, game_object.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that contains all the information for a game object entity including the sprite, actions, position, etc.  Should be inherited from to create different typesof game objects like units. 
*/

#ifndef __GAMEOBJECT_H
#define __GAMEOBJECT_H

#include "SFML\Graphics.hpp"
#include "game_texture.h"
#include "unit_action.h"
#include "overall_move_action.h"
#include "build_action.h"
#include "fog.h"
#include "world_tile.h"
#include <string>
#include <vector>
#include <queue>
#include "pathfinding.h"
#include <ctime>
#include "pickup.h"

class GameObject
{
private:

	//static int currentGameObjectId;

	int gridSize[2]; //How many squares on the game grid the object takes up
	int previousPos[2];
	int id;
	int position[2]; //Element 0 is x. Elememnt 1 is y
	float inSquareOffsetFromTileCenter[2]; //Offset for movement within its own tile
	sf::Sprite sprite; 
	sf::Sprite selectionSprite;
	float rotation; //Current rotation of the game object
	std::string type; //Might want to use in the future for inheritance
	float speed; //Movement speed
	float maxHealth;
	float currentHealth;

	int attackRange;
	GameObject* objectToAttack;
	float attackPower;
	float attackSpeed;
	float attackTimer;
	time_t timeOfLastAttack;

	bool isActive;
	bool isStructure;
	bool isPlayer;
	bool isFlying;

	//sf::Sprite visionMask;
	sf::CircleShape* visionMask;
	int vision;

	//float healthPercentage; Percentage or maxHealth and currentHealth?
	//Queue up actions that a unit will automatically execute. Example move to square 2, then 3, then 4, then attack. If queue empty will do nothing
	std::queue<unit_action*> actionQueue;

	//Pointer to the object the game object is currently carrying
	pickup* pickupCarried;
	const float TIME_FOR_PICKUP = 2.0;
	time_t timeLastPickup;

	bool beingBuilt;
	const int buildRate = 2;

	bool isCollecting;

	std::string createsType;

public:
	GameObject();
	GameObject(const bool isPlayer, const bool isStructure, const bool beingBuilt, const std::string type, int xPos, const int yPos, const int sizeX, const int sizeY, const int squareSize, sf::Texture* textureToSet, const float health, const float speed, sf::Texture* selectionTextureToSet, int vision, int attackRange, float attackPower, const float attackSpeed, const bool flying, std::string createsType); //xPos and yPos are the grid position. sizex and y are how many grid squares it takes up. square size is how big a grid square is

	Pathfinder *pathfinder;
	std::mutex pathLock;
	
	~GameObject();

	static int currentGameObjectId;
	const static int getNextGameObjectId();

	void cancelActions();
	void drawObject(sf::RenderWindow* windowToDrawTo);
	const bool busy();
	sf::Sprite& getSprite();
	sf::Sprite& getSelectionTexture();
	const int getGridPosX();
	const int getGridPosY();
	const int getGridSizeX();
	const int getGridSizeY();
	const int getID();
	const float getOffsetX();
	const float getOffsetY();
	//void move(const float x, const float y, const float squareSize);
	void moveWithPath(const int arraySize, int* path);
	void setRotation(const float rotation);
	void rotate(const float rotation);
	const float getRotation();
	void setTexture(sf::Texture* textureToSet); //Sets the sprites texture to the given texture. Would need to use for animation
	void doAction(const bool isPlayer, const int visionRange, const int worldSizeX, const int worldSizeY, const float squareSize, world_tile** backgroundTiles, Fog** fogOfWar);
	const bool getActive();
	const bool getFlying();
	const bool getStructure();
	void disable();
	sf::CircleShape* getVisionMask();
	//void setVisionMask(sf::Texture* textureToSet, int vision, float squareSize);
	void setVisionMask(int vision, float squareSize);
	float getCurrentHealth();
	float getMaxHealth();
	float getHealthPercentage();
	int getVision();
	std::string getType();
	const int getPreviousPosX();
	const int getPreviousPosY();
	const int getAttackRange();

	GameObject* getAttackTarget();
	void setAttackTarget(GameObject* attackTarget);

	const float getAttackPower();
	//return true if caused death
	bool doDamage(float damage);
	const float getAttackSpeed();
	const float getAttackTimer();
	void setAttackTimer(const float newTime);
	void incrementTimer(const float increment);
	bool tryAttack();
	bool getIsPlayer();

	pickup* getPickupCarried();
	void setPickupCarried(pickup* pickupToClaim);

	const int maxPickupCarried = 25;
	time_t getLastPickup();
	void resetLastPickup();
	int getTimeForPickup();
	const int getBuiltRate();
	const bool getBeingBuilt();

	float getPickupPercentage();

	void buildThisObject(GameObject* objToBuild);
	void queueUpBuildAction(GameObject* objToBuild, bool isPlayer);

	bool getIsCollecting();

	const float getSpeed();

	const std::string getCreatesType();

	const int getActionQueueSize();
};

#endif