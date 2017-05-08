#include "../include/interface_controller.h"



interface_controller::interface_controller()
{
}


interface_controller::~interface_controller()
{
}

bool interface_controller::determineAction(int windowSizeX, int windowSizeY, float mouseX, float mouseY, int* optionSelected, int resourceCount, int* currentlyDisplayedMenu, int trashIndex, float infoHeight, bool* newCam, int* newCamPosX, int* newCamPosY, int worldSizeX, int worldSizeY)
{
	//Determine if the player clicked the menu and if so, where
	if (mouseY > ((windowSizeY) - menuHeight - infoHeight) && mouseY < ((windowSizeY) - infoHeight) && mouseX > 0 && mouseX < windowSizeX)
	{
		int boxX = mouseX / ((windowSizeX) / MENU_SIZE_X);
		int boxY = (mouseY - (windowSizeY - menuHeight - infoHeight)) / ((menuHeight) / MENU_SIZE_Y);

		//See if a part of the minimap was clicked and send back where it was clicked to update the camera information
		//if (*currentlyDisplayedMenu == 0)
		{
			float miniMapTileSize = (float)menuHeight / (float)worldSizeY;

			if (mouseX < worldSizeX * miniMapTileSize && mouseX > 0 && mouseY < windowSizeY - infoHeight && mouseY > windowSizeY - infoHeight - menuHeight)
			{
				//Minimap was clicked, move the camera to it
				*newCam = true;
				*newCamPosX = mouseX / miniMapTileSize;
				*newCamPosY = (mouseY - (windowSizeY - infoHeight - menuHeight)) / miniMapTileSize;
			}
		}

		//Determine the appropriate action
		*optionSelected = boxY * MENU_SIZE_X + boxX;

		if (resourceCount < std::stoi(unitCostInfo->at(*optionSelected + *currentlyDisplayedMenu * MENU_SIZE_X * MENU_SIZE_Y)) || std::stoi(unitCostInfo->at(*optionSelected + *currentlyDisplayedMenu * MENU_SIZE_X * MENU_SIZE_Y)) < 0)
		{
			*optionSelected = -1;
		}
		else if (*optionSelected == trashIndex)
		{
			*currentlyDisplayedMenu = 0;
			return true;
		}
		//If the starting menu, change the menu to selected submenu. Also add check if not a selectable square
		else if (*currentlyDisplayedMenu == 0 && *optionSelected != trashIndex)
		{
			
			*currentlyDisplayedMenu = *optionSelected + 1;
			*optionSelected = -1;
		}
		//Back pressed
		else if (*currentlyDisplayedMenu != 0 && std::stoi(unitCostInfo->at(*optionSelected + *currentlyDisplayedMenu * MENU_SIZE_X * MENU_SIZE_Y)) == 0)
		{
			*currentlyDisplayedMenu = 0;
			*optionSelected = -1;
		}
		

		return true;
	}

	//Menu was not selected
	return false;
}

void interface_controller::setMenuHeight(float height)
{
	menuHeight = height;
}

void interface_controller::setUnitCostInfo(std::vector<std::string>* unitCostInfo)
{
	this->unitCostInfo = unitCostInfo;
}
