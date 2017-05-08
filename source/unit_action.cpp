#include "../include/unit_action.h"


unit_action::unit_action()
{
}


unit_action::~unit_action()
{
}

bool unit_action::executeActionOnUnit(bool isPlayer, int visionRange, int worldSizeX, int worldSizeY, int id, int* gridCordX, int* gridCordY, float* offsetX, float* offsetY, float speed, float* health, float squareSize, world_tile** backgroundTiles, Fog** fogOfWar, int* previousX, int* previousY, GameObject* actionOwner)
{
	return true;
}