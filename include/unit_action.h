/*
unit_action Class (unit_action.h, unit_action.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that classes inherit from in order to have units automattically execute actions. Game objects have a queue of these that are automattically executed when they are available. 
*/

#ifndef __UNITACTION_H
#define __UNITACTION_H

#include <string>
#include "world_tile.h"
#include "fog.h"

class unit_action
{
	//Class the actions will inherit from. Will have different actions to execute like move action
protected:
	std::string type;

public:
	unit_action();
	~unit_action();
	//Modifies the attributes of the unit depending on the type of action being used. Returns true if the action has been completed. False if not yet finished
	virtual bool executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner);
};

#endif