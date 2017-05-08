#include "../include/move_action.h"


move_action::move_action(int destinationX, int destinationY)
{
	cordToMoveToX = destinationX;
	cordToMoveToY = destinationY;
}


move_action::~move_action()
{
}

bool move_action::executeActionOnUnit(int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles)
{
	//Move the object inside its game world tile
	if (*gridCordX < cordToMoveToX)
	{
		//--->
		*offsetX += speed;
	}
	else if(*gridCordX > cordToMoveToX)
	{
		//<---
		*offsetX -= speed;
	}

	if (*gridCordY < cordToMoveToY)
	{
		//V
		*offsetY += speed;
	}
	else if (*gridCordY > cordToMoveToY)
	{
		//^
		*offsetY -= speed;
	}

	//Account for moving to the centre of the tile
	if (*gridCordX == cordToMoveToX && *offsetX < 0)
	{
		//--->
		*offsetX += speed;

		//Account for overshooting
		if (*offsetX > 0)
		{
			*offsetX = 0;
		}
	}
	else if (*gridCordX == cordToMoveToX && *offsetX > 0)
	{
		//<---
		*offsetX -= speed;

		if (*offsetX < 0)
		{
			*offsetX = 0;
		}
	}

	if (*gridCordY == cordToMoveToY && *offsetY < 0)
	{
		//V
		*offsetY += speed;

		if (*offsetY > 0)
		{
			*offsetY = 0;
		}
	}
	else if (*gridCordY == cordToMoveToY && *offsetY > 0)
	{
		//^
		*offsetY -= speed;

		if (*offsetY < 0)
		{
			*offsetY = 0;
		}
	}

	if (*offsetX > squareSize / 2)
	{
		*offsetX = -(squareSize / 2);
		*gridCordX = *gridCordX + 1;
	}
	if (*offsetX < -(squareSize / 2))
	{
		*offsetX = (squareSize / 2);
		*gridCordX = *gridCordX - 1;
	}
	if (*offsetY > squareSize / 2)
	{
		*offsetY = -(squareSize / 2);
		*gridCordY = *gridCordY + 1;
	}
	if (*offsetY < -(squareSize / 2))
	{
		*offsetY = (squareSize / 2);
		*gridCordY = *gridCordY - 1;
	}

	//Check if its destination has been reached
	if (*gridCordX == cordToMoveToX && *gridCordY == cordToMoveToY && *offsetX == 0 && *offsetY == 0)
	{
		return true; //Action has been completed
	}

	return false;
}

int move_action::getDestX()
{
	return cordToMoveToX;
}
int move_action::getDestY()
{
	return cordToMoveToY;
}
