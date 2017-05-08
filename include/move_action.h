/*
move_action Class (move_action.h, move_action.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that the overall_move_action class uses to make tile to tile movements for game objects. 
*/

#ifndef __MOVEACTION_H
#define __MOVEACTION_H

#include "world_tile.h"

class move_action
{
private:
	int cordToMoveToX;
	int cordToMoveToY;

public:
	move_action(int destinationX, int destinationY);
	~move_action();
	bool executeActionOnUnit(int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles);
	int getDestX();
	int getDestY();
};

#endif