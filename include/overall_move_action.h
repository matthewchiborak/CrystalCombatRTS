/*
overall_move_action Class (overall_move_action.h, overall_move_action.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Classs that is a type of unit action. Used by game objects to automattically move. Uses move_action to make tile to tile movements
*/

#ifndef __OVERALLMOVEACTION_H
#define __OVERALLMOVEACTION_H

#include "unit_action.h"
#include "move_action.h"
#include "fog.h"
#include <queue>
#include <math.h>  


class overall_move_action : public unit_action
{
private:
	std::queue<move_action*> moveActions;
	bool movementStarted;
public:
	overall_move_action(int arrayLength, int* cordPositions);
	~overall_move_action();
	bool executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner);
};

#endif