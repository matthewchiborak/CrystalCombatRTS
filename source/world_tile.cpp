#include "../include/world_tile.h"
#include "../include/game_object.h"

#include <iostream>


world_tile::world_tile()
{
	//Unassigned data
	occupiedId = -1;
	position[0] = -1;
	position[1] = -1;
	// Default cost
	cost = DEFAULT_TILE_COST;
	//occupiedObj = nullptr;
	tilePickup = NULL;
	createsResources = false;
	timeOfLastSpawn = time(0);

	
}


world_tile::~world_tile()
{
}

void world_tile::setTextureToLast(sf::Texture* textureToSet, float squareSize, int xCord, int yCord)
{
	lastObjectOnIt.setTexture(*textureToSet);
	lastObjectOnIt.setOrigin(textureToSet->getSize().x / 2.0, textureToSet->getSize().y / 2.0);


	lastObjectOnIt.setPosition(sf::Vector2f(xCord * (squareSize)+(squareSize / 2.0), yCord * (squareSize)+(squareSize / 2.0)));
	lastObjectOnIt.setScale(sf::Vector2f((float)1 * (squareSize) / textureToSet->getSize().x, (float)1 * (squareSize) / (float)textureToSet->getSize().y));
}

void world_tile::copyASpriteToTheLastSeen(sf::Sprite copiedSprite)
{
	lastObjectOnIt = copiedSprite;
}

sf::Sprite* world_tile::getSprite()
{
	return &sprite;
}

int world_tile::getOccupiedId()
{
	return occupiedId;
}

void world_tile::setPosition(const int x, const int y)
{
	position[0] = x;
	position[1] = y;
}

const int world_tile::getX()
{
	return position[0];
}

const int world_tile::getY()
{
	return position[1];
}


bool world_tile::tryClaimTile(GameObject* objToClaim)
{
	if (occupiedObj.size() == 0)
	{
		//occupiedObj = objToClaim;
		occupiedObj.push_back(objToClaim);
		return true;
	}
	else //if (objToClaim->getIsPlayer() && occupiedObj[0]->getIsPlayer() && !occupiedObj[0]->getStructure())
	{
		//Only push back if claimed by the same player's object and it isnt a structure
		occupiedObj.push_back(objToClaim);
		return true;
	}
	/*else if (!objToClaim->getIsPlayer() && !occupiedObj[0]->getIsPlayer() && !occupiedObj[0]->getStructure())
	{
		occupiedObj.push_back(objToClaim);
		return true;
	}*/

	return false;
}

void world_tile::freeTile(GameObject* objectLeaving)
{
	//occupiedId = -1;
	for (int i = 0; i < occupiedObj.size(); i++)
	{
		if (occupiedObj[i]->getID() == objectLeaving->getID())
		{
			occupiedObj.erase(occupiedObj.begin() + i);
			break;
		}
	}
}

const int world_tile::getCost()
{
	return cost;
}

void world_tile::setCost(const int cost)
{
	this->cost = cost;
}


GameObject* world_tile::getOccupiedObj()
{
	if (occupiedObj.size() > 0)
	{
		return occupiedObj[0];
	}

	return NULL;
}
void world_tile::setOccupiedObj(GameObject* objToSet)
{
	//occupiedObj = objToSet;
	occupiedObj.push_back(objToSet);
}

pickup* world_tile::claimPickupIfExists()
{
	if (tilePickup)
	{
		int valueTaking = 1;

		if (tilePickup->getValue() > valueTaking)
		{
			pickup* temp = new pickup(pickupTexture, pickupSize, valueTaking);
			tilePickup->addToValue(-1 * valueTaking);
			return temp;
		}
		else
		{
			pickup* temp = tilePickup;
			tilePickup = nullptr;
			return temp;
		}

	}

	//Didn't have one
	return nullptr;
}

pickup* world_tile::checkPickup()
{
	return tilePickup;
}

void world_tile::setResourceGeneration(const bool status)
{
	createsResources = status;
}

void world_tile::tryCreateResource(sf::Texture* pickupTexture, float size)
{
	if (createsResources)
	{
		//Create the resource if it doesn't already exist based on time past since last generation
		if (time(0) - timeOfLastSpawn > TILE_GENERATION_TIME)
		{
			//Only if doesn't alreayd have one
			if (!tilePickup)
			{
				tilePickup = new pickup(pickupTexture, size, RESOURCE_VALUE);
				tilePickup->getSprite()->setPosition(position[0] * size, position[1] * size);
				timeOfLastSpawn = time(0);
			}
		}
	}
}

void world_tile::createResources(sf::Texture* pickupTexture, float size)
{
	tilePickup = new pickup(pickupTexture, size, RESOURCE_VALUE);
	tilePickup->getSprite()->setPosition(position[0] * size, position[1] * size);

	this->pickupTexture = pickupTexture; 
	pickupSize = size;
}

bool world_tile::hasResource()
{
	if (tilePickup)
	{
		return true;
	}

	return false;
}

sf::Sprite* world_tile::getResourceSprite()
{
	return tilePickup->getSprite();
}

bool world_tile::checkIfThisObjInHere(GameObject* objToCheckFor)
{
	for (int i = 0; i < occupiedObj.size(); i++)
	{
		if (objToCheckFor->getID() == occupiedObj[i]->getID())
		{
			return true;
		}
	}

	return false;
}

int world_tile::numberOfObjectsOnMe()
{
	return occupiedObj.size();
}
GameObject* world_tile::getObjectOnMeWithIndex(int index)
{
	if (index < occupiedObj.size())
	{
		return occupiedObj[index];
	}

	return NULL;
}