/*
build_action Class (build_action.h, build_action.cpp)

Created: March 1, 2017
Author: Matthew Chiborak

Description:
Actions that an object that is building another object executes
*/

#ifndef __BUILDACTION_H
#define __BUILDACTION_H

#include "unit_action.h"
#include <cmath>

class build_action : public unit_action
{
private:
	GameObject* objBeingBuilt;
	time_t lastTimeBuild;
public:
	build_action(GameObject* objBeingBuilt);
	~build_action();
	bool executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner);
};

#endif