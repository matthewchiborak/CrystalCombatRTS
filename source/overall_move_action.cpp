#include "../include/overall_move_action.h"
#include "../include/game_object.h"
#include "../include/Renderer.h"

overall_move_action::overall_move_action(int arrayLength, int* cordPositions)
{
	for (int i = 0; i < arrayLength; i+=2)
	{
		moveActions.push(new move_action(cordPositions[i], cordPositions[i + 1]));
	}

	movementStarted = false;
	type = "move";
}


overall_move_action::~overall_move_action()
{
}

bool overall_move_action::executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner)
{
	if (moveActions.empty())
	{
		return true;
	}

	//Claim the first movement
	if (!movementStarted)
	{
		movementStarted = true;
		//Try claim the id
		if (backgroundTiles[moveActions.front()->getDestX()][moveActions.front()->getDestY()].tryClaimTile(actionOwner))
		{
			//Update
			*previousX = *gridCordX;
			*previousY = *gridCordY;

			backgroundTiles[*gridCordX][*gridCordY].freeTile(actionOwner);

			*gridCordX = moveActions.front()->getDestX();
			*gridCordY = moveActions.front()->getDestY();

			//if theres a resources on the tile it just claimed, pick it up
			if (actionOwner->getType() == "worker")
			{
				if (!actionOwner->getPickupCarried() || actionOwner->getPickupCarried()->getValue() < actionOwner->maxPickupCarried)
				{
					pickup* pickupToClaim = backgroundTiles[moveActions.front()->getDestX()][moveActions.front()->getDestY()].claimPickupIfExists();

					if (pickupToClaim)
					{
						//If already had a resource, combine them
						if (actionOwner->getPickupCarried())
						{
							actionOwner->getPickupCarried()->addToValue(pickupToClaim->getValue());
							delete pickupToClaim;
						}
						else
						{
							actionOwner->setPickupCarried(pickupToClaim);
						}
					}
				}
			}
		}
		else
		{
			//Cancel
			while (!moveActions.empty())
			{
				delete moveActions.front();
				moveActions.pop();
			}

			return true;
		}
	}

	if (moveActions.front()->executeActionOnUnit(previousX, previousY, offsetX, offsetY, speed, health, squareSize, backgroundTiles))
	{
		delete moveActions.front();
		moveActions.pop();

		// Check if in attack range
		if (attackTargetInRange(actionOwner, actionOwner->getAttackTarget()))
		{
			while (!moveActions.empty())
				moveActions.pop();
		}

		//If no more move actions end the action
		if (moveActions.empty())
		{
			return true;
		}

		//If something is now occupying the next space, cancel the currentMoveOrder. Maybe change this later to refind a new path. If succeed with claim the tile so must release the previous
	
		if (backgroundTiles[moveActions.front()->getDestX()][moveActions.front()->getDestY()].tryClaimTile(actionOwner))
		{
			//Update current position and previous position
			*previousX = *gridCordX;
			*previousY = *gridCordY;

			backgroundTiles[*gridCordX][*gridCordY].freeTile(actionOwner);

			*gridCordX = moveActions.front()->getDestX();
			*gridCordY = moveActions.front()->getDestY();

		
			//Free the tile it is leaving
			
			//if theres a resources on the tile it just claimed, pick it up
			if (actionOwner->getType() == "worker")
			{
				pickup* pickupToClaim = backgroundTiles[moveActions.front()->getDestX()][moveActions.front()->getDestY()].claimPickupIfExists();


				if (pickupToClaim)
				{
					//If already had a resource, combine them
					if (actionOwner->getPickupCarried())
					{
						actionOwner->getPickupCarried()->addToValue(pickupToClaim->getValue());
						delete pickupToClaim;
					}
					else
					{
						actionOwner->setPickupCarried(pickupToClaim);
					}
				}
			}
			
			return false;
		}
		else //Cancel order
		{
			while (!moveActions.empty())
			{
				delete moveActions.front();
				moveActions.pop();
			}

			return true;
		}
	}

	return false;
}
