/*
interface_controller Class (interface_controller.h, interface_controller.cpp)

Created: Jan 27, 2017
Author: Matthew Chiborak

Description:
Class that will take the users mouse position on the screen and will determine what it is that the user is clicking on the HUD.  The HUD will be on a 
separate view so it won't be affected by the the zooming or move of the camera. It will be static at the bottom of the screen
*/


#ifndef __INTERFACECONTROLLER_H
#define __INTERFACECONTROLLER_H

#include <iostream>
#include <vector>
#include <string>

#define MENU_SIZE_X 10
#define MENU_SIZE_Y 2

class interface_controller
{
private: 
	float menuHeight;
	std::vector<std::string>* unitCostInfo;

public:
	interface_controller();
	~interface_controller();

	//Not sure how handling decisions once made. TODO this later. returns true to menu was clicked. False if otherwise
	//The Option selected is what unit was selected. If it is a negative value, that means that the player does not have enough fund and so the object is not selected
	bool determineAction(int windowSizeX, int windowSizeY, float mouseX, float mouseY, int* optionSelected, int resourceCount, int* currentlyDisplayedMenu, int trashIndex, float infoHeight, bool* newCam, int* newCamPosX, int* newCamPosY, int worldSizeX, int worldSizeY);
	void setMenuHeight(float height);
	void setUnitCostInfo(std::vector<std::string>* unitCostInfo);
};

#endif