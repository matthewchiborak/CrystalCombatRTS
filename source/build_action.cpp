#include "../include/build_action.h"
#include "../include/game_object.h"


build_action::build_action(GameObject* objBeingBuilt)
{
	type = "build";
	this->objBeingBuilt = objBeingBuilt;
	lastTimeBuild = time(0);
}


build_action::~build_action()
{
}

bool build_action::executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner)
{
	//Check if building has finished
	if (objBeingBuilt)
	{
		if (!objBeingBuilt->getBeingBuilt())
		{
			//Building has finished
			return true;
		}
	}
	else
	{
		return true;
	}

	//See if enough time has elapsed from the last time it was built towards
	if (time(0) - lastTimeBuild > actionOwner->getBuiltRate())
	{
		lastTimeBuild = time(0);

		if (objBeingBuilt)
		{
			//Must be beside it. Check all the tiles around it
			
			bool foundTheBuilder = false;
			
			//Top
			for (int i = objBeingBuilt->getGridPosX() - 1; i < objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX() + 1; i++)
			{
				if (i >= 0 && i < worldSizeX && objBeingBuilt->getGridPosY() - 1 >= 0 && objBeingBuilt->getGridPosY() < worldSizeY)
				{
					//if (backgroundTiles[i][objBeingBuilt->getGridPosY() - 1].getOccupiedObj())
					if(backgroundTiles[i][objBeingBuilt->getGridPosY() - 1].checkIfThisObjInHere(actionOwner))
					{
						//if (backgroundTiles[i][objBeingBuilt->getGridPosY() - 1].getOccupiedObj()->getID() == actionOwner->getID())
						{
							foundTheBuilder = true;
							break;
						}
					}
				}
			}

			//Bottom
			if (!foundTheBuilder)
			{
				for (int i = objBeingBuilt->getGridPosX() - 1; i < objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX() + 1; i++)
				{
					if (i >= 0 && i < worldSizeX && objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY() >= 0 && objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY() < worldSizeY)
					{
						//if (backgroundTiles[i][objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY()].getOccupiedObj())
						if(backgroundTiles[i][objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY()].checkIfThisObjInHere(actionOwner))
						{
							//if (backgroundTiles[i][objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY()].getOccupiedObj()->getID() == actionOwner->getID())
							{
								foundTheBuilder = true;
								break;
							}
						}
					}
				}
			}

			//Left
			if (!foundTheBuilder)
			{
				for (int i = objBeingBuilt->getGridPosY() - 1; i < objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY() + 1; i++)
				{
					if (i >= 0 && i < worldSizeY && objBeingBuilt->getGridPosX() - 1 >= 0 && objBeingBuilt->getGridPosX() - 1 < worldSizeX)
					{
						//if (backgroundTiles[objBeingBuilt->getGridPosX() - 1][i].getOccupiedObj())
						if(backgroundTiles[objBeingBuilt->getGridPosX() - 1][i].checkIfThisObjInHere(actionOwner))
						{
							//if (backgroundTiles[objBeingBuilt->getGridPosX() - 1][i].getOccupiedObj()->getID() == actionOwner->getID())
							{
								foundTheBuilder = true;
								break;
							}
						}
					}
				}
			}

			//Right
			if (!foundTheBuilder)
			{
				for (int i = objBeingBuilt->getGridPosY() - 1; i < objBeingBuilt->getGridPosY() + objBeingBuilt->getGridSizeY() + 1; i++)
				{
					if (i >= 0 && i < worldSizeY && objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX() >= 0 && objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX() < worldSizeX)
					{
						//if (backgroundTiles[objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX()][i].getOccupiedObj())
						if(backgroundTiles[objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX()][i].checkIfThisObjInHere(actionOwner))
						{
							//if (backgroundTiles[objBeingBuilt->getGridPosX() + objBeingBuilt->getGridSizeX()][i].getOccupiedObj()->getID() == actionOwner->getID())
							{
								foundTheBuilder = true;
								break;
							}
						}
					}
				}
			}

			if(foundTheBuilder)
			{
				objBeingBuilt->doDamage(-10);

				if (!objBeingBuilt->getBeingBuilt())
				{
					//Building has finished
					return true;
				}
			}
				
		}
		else
		{
			//Obj is no longer there for some reason
			return true;
		}
	}

	//Not done building yet
	return false;
}
