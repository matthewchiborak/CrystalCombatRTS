#include "../include/game_object.h"

int GameObject::currentGameObjectId = 0;

const int GameObject::getNextGameObjectId()
{
	return currentGameObjectId;
}

GameObject::GameObject()
{

}

GameObject::GameObject(const bool isPlayer, const bool isStructure, const bool beingBuilt, const std::string type, const int xPos, const int yPos, const int sizeX, const int sizeY, const int squareSize, sf::Texture* textureToSet, const float health, const float speed, sf::Texture* selectionTextureToSet, int vision, int attackRange, float attackPower, const float attackSpeed, const bool flying, std::string createsType)
{
	this->isPlayer = isPlayer;
	rotation = 0;
	this->type = type;
	this->createsType = createsType;
	position[0] = xPos;
	position[1] = yPos;
	previousPos[0] = xPos;
	previousPos[1] = yPos;
	gridSize[0] = sizeX;
	gridSize[1] = sizeY;
	maxHealth = health;
	currentHealth = health;
	this->vision = vision;
	this->speed = speed;
	isActive = true;
	this->isStructure = isStructure;
	inSquareOffsetFromTileCenter[0] = 0;
	inSquareOffsetFromTileCenter[1] = 0;
	sprite.setTexture(*textureToSet);
	selectionSprite.setTexture(*selectionTextureToSet);
	id = currentGameObjectId++;
	sprite.setOrigin(textureToSet->getSize().x / 2.0, textureToSet->getSize().y / 2.0);
	sprite.setScale(sf::Vector2f(getGridSizeX() * (squareSize) / textureToSet->getSize().x, getGridSizeY() * (squareSize) / textureToSet->getSize().y));

	objectToAttack = NULL;

	this->attackRange = attackRange;
	this->attackPower = attackPower;
	this->attackSpeed = attackSpeed;
	this->attackTimer = 0;
	timeOfLastAttack = time(0);

	selectionSprite.setOrigin(selectionTextureToSet->getSize().x / 2.0, selectionTextureToSet->getSize().y / 2.0);
	selectionSprite.setScale(sf::Vector2f(getGridSizeX() * (squareSize) / selectionTextureToSet->getSize().x, getGridSizeY() * (squareSize) / selectionTextureToSet->getSize().y));

	pathfinder = NULL;
	visionMask = NULL;

	pickupCarried = NULL;
	timeLastPickup = time(0);

	isFlying = flying;

	this->beingBuilt = beingBuilt;

	isCollecting = false;

	if (beingBuilt)
	{
		currentHealth = 1;
	}
}

GameObject::~GameObject()
{
	
}

void GameObject::cancelActions()
{
	while (!actionQueue.empty())
		actionQueue.pop();
}

void GameObject::drawObject(sf::RenderWindow* windowToDrawTo)
{
	windowToDrawTo->draw(sprite);
}

const bool GameObject::busy()
{
	return (pathfinder || !actionQueue.empty() || beingBuilt);
}

sf::Sprite& GameObject::getSprite()
{
	return sprite;
}

sf::Sprite& GameObject::getSelectionTexture()
{
	return selectionSprite;
}

const int GameObject::getGridPosX()
{
	return position[0];
}
const int GameObject::getGridPosY()
{
	return position[1];
}

const int GameObject::getGridSizeX()
{
	return gridSize[0];
}
const int GameObject::getGridSizeY()
{
	return gridSize[1];
}

const int GameObject::getID()
{
	return id;
}

void GameObject::disable()
{
	isActive = false;
}

void GameObject::moveWithPath(const int arraySize, int* path)
{
	//Cancel the current action
	while (!actionQueue.empty())
		actionQueue.pop();
	

	//If its being built it cannot be given an action just yet
	if (!beingBuilt)
	{
		actionQueue.push(new overall_move_action(arraySize, path));
	}
}

const float GameObject::getOffsetX()
{
	return inSquareOffsetFromTileCenter[0];
}

const float GameObject::getOffsetY()
{
	return inSquareOffsetFromTileCenter[1];
}

void GameObject::setRotation(const float rotation)
{
	this->rotation = rotation;
}

void GameObject::rotate(const float rotation)
{
	this->rotation += rotation;

	if (this->rotation >= 360)
	{
		this->rotation -= 360;
	}
	if (this->rotation < 0)
	{
		this->rotation += 360;
	}
}

const float GameObject::getRotation()
{
	return rotation;
}

void GameObject::setTexture(sf::Texture* textureToSet)
{
	sprite.setTexture(*textureToSet);
}


void GameObject::doAction(const bool isPlayer, const int visionRange, const int worldSizeX, const int worldSizeY, const float squareSize, world_tile** backgroundTiles, Fog** fogOfWar)
{
	//If currently on a tile that has resources and is a worker, claim its resources
	if (getType() == "worker")
	{
		if (backgroundTiles[getGridPosX()][getGridPosY()].checkPickup())
		{
			isCollecting = true;

			if (!getPickupCarried() || getPickupCarried()->getValue() < maxPickupCarried)
			{
				
				if (time(0) - getLastPickup() > getTimeForPickup())
				{
					resetLastPickup();

					pickup* pickupToClaim = backgroundTiles[getGridPosX()][getGridPosY()].claimPickupIfExists();

					if (pickupToClaim)
					{
						//If already had a resource, combine them
						if (getPickupCarried())
						{
							getPickupCarried()->addToValue(pickupToClaim->getValue());
							delete pickupToClaim;
						}
						else
						{
							setPickupCarried(pickupToClaim);
						}
					}
				}
			}
		}
		else
		{
			isCollecting = false;
		}
	}

	//If there's action available to do, do it
	if (!actionQueue.empty())
	{
		if (actionQueue.front()->executeActionOnUnit(isPlayer, this->vision, worldSizeX, worldSizeY, id, &position[0], &position[1], &inSquareOffsetFromTileCenter[0], &inSquareOffsetFromTileCenter[1], speed, &currentHealth, squareSize, backgroundTiles, fogOfWar, &previousPos[0], &previousPos[1], this))
		{
			//Action is finished. Remove it from the queue.
			delete actionQueue.front();
			actionQueue.pop();
		}
	}
}

const bool GameObject::getActive()
{
	return isActive;
}

const bool GameObject::getFlying()
{
	return isFlying;
}

const bool GameObject::getStructure()
{
	return isStructure;
}

sf::CircleShape* GameObject::getVisionMask()
{
	return visionMask;
}


void GameObject::setVisionMask(int vision, float squareSize)
{
	if (visionMask)
		delete visionMask;

	visionMask = new sf::CircleShape((squareSize * (vision * 2 + 1)) / 2);

	visionMask->setFillColor(sf::Color::Magenta);

	
	visionMask->setOrigin((squareSize * (vision * 2 + 1)) / 2, (squareSize * (vision * 2 + 1)) / 2);
}

float GameObject::getCurrentHealth()
{
	return currentHealth;
}
float GameObject::getMaxHealth()
{
	return maxHealth;
}
float GameObject::getHealthPercentage()
{
	return currentHealth / maxHealth;
}

int GameObject::getVision()
{
	return vision;
}

std::string GameObject::getType()
{
	return type;
}

const int GameObject::getPreviousPosX()
{
	return previousPos[0];
}
const int GameObject::getPreviousPosY()
{
	return previousPos[1];
}
const int GameObject::getAttackRange()
{
	return attackRange;
}

GameObject* GameObject::getAttackTarget()
{
	return objectToAttack;
}
void GameObject::setAttackTarget(GameObject* attackTarget)
{
	objectToAttack = attackTarget;
}

const float GameObject::getAttackPower()
{
	return attackPower;
}
bool GameObject::doDamage(float damage)
{
	currentHealth -= damage;

	if (currentHealth > maxHealth)
	{
		currentHealth = maxHealth;
		beingBuilt = false;
	}

	//If dead
	if (currentHealth <= 0)
	{
		// Allow cleanup handled in renderer
		currentHealth = 0;
		return true;
	}

	return false;
}

const float GameObject::getAttackSpeed()
{
	return attackSpeed;
}
const float GameObject::getAttackTimer()
{
	return attackTimer;
}
void GameObject::setAttackTimer(const float newTime)
{
	attackTimer = newTime;
}
void GameObject::incrementTimer(const float increment)
{
	attackTimer += increment;
}

bool GameObject::tryAttack()
{
	if (time(0) - timeOfLastAttack > attackSpeed)
	{
		//Been long enough time to attack.
		timeOfLastAttack = time(0);
		return true;
	}

	return false;
}

bool GameObject::getIsPlayer()
{
	return isPlayer;
}

pickup* GameObject::getPickupCarried()
{
	return pickupCarried;
}
void GameObject::setPickupCarried(pickup* pickupToClaim)
{
	pickupCarried = pickupToClaim;
}

time_t GameObject::getLastPickup()
{
	return timeLastPickup;
}
void GameObject::resetLastPickup()
{
	timeLastPickup = time(0);
}

int GameObject::getTimeForPickup()
{
	return TIME_FOR_PICKUP;
}

const int GameObject::getBuiltRate()
{
	return buildRate;
}

const bool GameObject::getBeingBuilt()
{
	return beingBuilt;
}

void GameObject::buildThisObject(GameObject* objToBuild)
{
	while (!actionQueue.empty())
		actionQueue.pop();

	//If its being built it cannot be given an action just yet
	if (!beingBuilt)
	{
		actionQueue.push(new build_action(objToBuild));
	}
}

void GameObject::queueUpBuildAction(GameObject* objToBuild, bool isPlayer)
{
	if (objToBuild->getIsPlayer() && isPlayer)
	{
		if (objToBuild->getBeingBuilt())
		{
			if (!beingBuilt)
			{
				actionQueue.push(new build_action(objToBuild));
			}
		}
	}

	if (!objToBuild->getIsPlayer() && !isPlayer)
	{
		if (objToBuild->getBeingBuilt())
		{
			if (!beingBuilt)
			{
				actionQueue.push(new build_action(objToBuild));
			}
		}
	}
}

bool GameObject::getIsCollecting()
{
	return isCollecting;
}

float GameObject::getPickupPercentage()
{
	return (time(0) - timeLastPickup) / TIME_FOR_PICKUP;
}

const float GameObject::getSpeed()
{
	return speed;
}

const std::string GameObject::getCreatesType()
{
	return createsType;
}

const int GameObject::getActionQueueSize()
{
	return actionQueue.size();
}