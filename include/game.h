/*
Game Class (game.h, game_main.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that acts as the main for the game thread
*/

#ifndef __GAME_H
#define __GAME_H

#include <SFML/Graphics.hpp>
#include <concurrent_queue.h>
#include <mutex>
#include "lock.h"
#include "texture_manager.h"
#include "renderer.h"
#include "game_object.h"
#include "text_parser.h"
#include "sleeper.h"
#include "interface_controller.h"

#include <queue>


class Game;

void game_begin(int *status, Lock *lock, Game *game);
void game_logic(int *status, Game* game, Lock *lock);
void game_render(int *status, Game* game, sf::RenderWindow* window);

struct scores
{
	int units, structures, resources;

	scores()
	{
		units = 0;
		structures = 0;
		resources = 0;
	}
};

enum Action
{
	None = -1,
	Idle = 0,
	Attack = 1,
	Move = 2,
	Scout = 3,
	Build = 4,
	Delete = 5,
	Farm = 6
};

enum Menu
{
	none = 0,
	base = 1,
	worker = 2
};

class AgentAction
{
public:
	// Type of action to perform
	Action type;
	// IDs of action issuers (base or moving units)
	std::vector<int> ids;
	// Attack target id
	int target;
	// Move target tile (In grid)
	sf::Vector2i targetTile;
	// Build info
	int buildMenu, buildOption;
public:
	AgentAction()
	{
		target = -1;
		targetTile.x = -1;
		targetTile.y = -1;
		type = Action::None;
	}
};

class Game
{
private:
	std::queue<sf::Event> events;
public:
	// Initiates pathfinding
	static void moveToLocation(GameObject *object, PathingList &pathList, world_tile *startTile, world_tile *endTile)
	{
		// Check inputs
		if (!object || !startTile || !endTile)
		{
			//Logger::log_warning(std::cout, "Move To Location failed - one or more parameters were null");
			return;
		}

		// Get lock
		while (!object->pathLock.try_lock());
		if (!object->pathfinder)
		{
			object->pathfinder = new Pathfinder(
				startTile->getX(), startTile->getY(), endTile->getX(), endTile->getY(),
				&object->pathLock, 0, object->getFlying());
		}
		else
		{
			// No longer need old pathfinder - clear object
			object->pathLock.unlock();
			delete object->pathfinder;
			// Waits for the pathfinder to finish deleting then create new one
			while (!object->pathLock.try_lock());
			object->pathfinder = new Pathfinder(
				startTile->getX(), startTile->getY(), endTile->getX(), endTile->getY(),
				&object->pathLock, 0, object->getFlying());
		}

		// Add to list of pathfinders
		pathList.add(object);
		object->pathLock.unlock();
	}
public:
	Renderer* renderer; //The render function should be static
	interface_controller interfaceController;

	// Agent actions to perform
	concurrency::concurrent_queue<AgentAction*> aiActions;

	// Are we performing a concurrent action?
	bool aiProcessing = false;

	// Is the game initialized?
	bool ready;

	// Perceived capabilities
	scores perceivedPlayer, perceivedOpponent;
	scores actualPlayer, actualOpponent;

	// Game constants
	const int TILE_SIZE = 32;
	const float SCREEN_EDGE_SCROLL_LENGTH = 20;
	const float CAMERA_MOVE_SPEED = 0.0005;
	const float ZOOM_SPEED = 0.025;
	const float MAX_ZOOM = 3.0;
	const float MIN_ZOOM = 0.5;
	const int TRASH_INDEX = 19;
	const float UNIT_INFO_BOX_HEIGHT = 40;
	const int PICKUP_GEN_INDEX = 2;
	const long long FRAME_TIME = (1000 / 60);

	// Render variables
	int GAME_WORLD_SIZE_X;
	int GAME_WORLD_SIZE_Y;	

	float videoModeX;
	float videoModeY;

	bool focus;
	int selectedAction;
	bool actionSelected;

	//Selection for the AI
	GameObject* opponentObjectCurrentlySelected;
	bool opponentObjectSelected;

	//The index of the last clicked menu option
	int menuOptionSelected;
	int currentMenuDisplayed;

	sf::Vector2i mousePos;
	//Start position for click and drag box
	sf::Vector2i mouseStartPos;
	bool mouseDown;
	std::vector<GameObject*> currentMultiSelectedObjects;
	int singledOutMultiSelectedObjectInd;

	//Lock windowLock;
	std::mutex windowLock;

	//Lock eventLock
	std::mutex eventLock;

	//Resource couts for the players
	int playerResourceCount;
	int opponentResourceCount;
	sf::Font* menuFont;
	std::vector<std::string>* unitCostInfo;
	std::vector<std::string>* unitCreationInfo;
	TextParser textParser;

	// Events
	void addEvent(const sf::Event event) { events.push(event); };
	const sf::Event popEvent() { sf::Event e = events.front(); events.pop(); return e; };
	const unsigned int eventCount() { return events.size(); };

	//Selecting an object
	bool arrowPressed;

	// Pathfinding list
	PathingList pathList;

	//Game time duration
	const int TIME_FOR_GAME = 3600; //In seconds
	time_t timeStartedGame;
	time_t timeOfPause;

	//Strings for the killfeed
	std::string killFeedStrings[6];
	int numberOfKillFeedStrings;

	//Bool to keep track if the game is paused and the game is over
	bool paused;
	bool gameIsOver;
	bool playerWins;
	bool gameHasStarted;

public:
	Game() 
	{ 
		ready = false;
		renderer = NULL; 
		videoModeX = 800;
		videoModeY = 600;
		actionSelected = false;
		playerResourceCount = 1000;
		opponentResourceCount = 1000;
		menuOptionSelected = -1;
		currentMenuDisplayed = 0;
		mouseDown = false;
		mousePos.x = 0;
		mousePos.y = 0;
		mouseStartPos.x = 0;
		mouseStartPos.y = 0;
		singledOutMultiSelectedObjectInd = -1; //Not selected
		arrowPressed = false;
		timeStartedGame = time(0);
		timeOfPause = time(0);

		paused = true;
		gameIsOver = false;
		playerWins = false;
		gameHasStarted = false;

		numberOfKillFeedStrings = 6;

		for (int i = 0; i < numberOfKillFeedStrings; i++)
		{
			killFeedStrings[i] = "";
		}
	};
	~Game() 
	{ 
		// Clear aiActions
		AgentAction *deleteAction = NULL;
		while (!aiActions.empty())
		{
			while (!aiActions.try_pop(deleteAction));
			delete deleteAction;
		}

		//This is causing the stack overflow
		if (renderer) delete renderer; 
	};

	void killGameObject(GameObject *object)
	{
		// If it is selected, deselect it
		for (int i = 0; i < currentMultiSelectedObjects.size(); i++)
		{
			if (object == currentMultiSelectedObjects[i])
			{
				currentMultiSelectedObjects.erase(currentMultiSelectedObjects.begin() + i);
				if (i == singledOutMultiSelectedObjectInd)
					singledOutMultiSelectedObjectInd = -1;
				break;
			}
		}

		// Clean up everything
		if (object->pathfinder)
		{
			while (!object->pathLock.try_lock());
			pathList.remove(object);
			object->pathLock.unlock();
			if(object->pathfinder)
				delete object->pathfinder;
		}

		// Remove object
		delete object;
	};

	GameObject * createUnit()
	{
		if (currentMultiSelectedObjects.size() == 0)
			return NULL;

		return createUnit(currentMultiSelectedObjects[0], true);
	}

	GameObject * createUnit(GameObject *creator, const bool isPlayer, const int mockMenuOption = -1, const int mockMenuDisplayed = -1)
	{
		GameObject * createdUnit = NULL;
		world_tile* wantedTile = NULL;

		int optionSelected, menuDisplayed;

		if (isPlayer)
		{
			optionSelected = menuOptionSelected;
			menuDisplayed = currentMenuDisplayed;
		}
		else
		{
			optionSelected = mockMenuOption;
			menuDisplayed = mockMenuDisplayed;
		}


		//Search top and bottom
		for (int i = -1; i < creator->getGridSizeX() + 1; i++)
		{
			//If in bounds
			if (i + creator->getGridPosX() >= 0 && creator->getGridPosY() - 1 >= 0)
			{
				if (i + creator->getGridPosX() < GAME_WORLD_SIZE_X && creator->getGridPosY() - 1 < GAME_WORLD_SIZE_Y)
				{
					wantedTile = renderer->getTile(i + creator->getGridPosX(), creator->getGridPosY() - 1);

					createdUnit = createUnit(wantedTile, isPlayer, optionSelected, menuDisplayed);

					if (createdUnit)
					{
						break;
					}
				}
			}
		}
		//Bottom
		if (!createdUnit)
		{
			for (int i = -1; i < creator->getGridSizeX() + 1; i++)
			{
				//If in bounds
				if (i + creator->getGridPosX() >= 0 && creator->getGridPosY() + creator->getGridSizeY() >= 0)
				{
					if (i + creator->getGridPosX() < GAME_WORLD_SIZE_X && creator->getGridPosY() + creator->getGridSizeY() < GAME_WORLD_SIZE_Y)
					{
						wantedTile = renderer->getTile(i + creator->getGridPosX(), creator->getGridPosY() + creator->getGridSizeY());

						createdUnit = createUnit(wantedTile, isPlayer, optionSelected, menuDisplayed);

						if (createdUnit)
						{
							break;
						}
					}
				}
			}
		}

		//Search left and right side
		//Left
		if (!createdUnit)
		{
			for (int i = -1; i < creator->getGridSizeY() + 1; i++)
			{
				//If in bounds
				if (i + creator->getGridPosY() >= 0 && creator->getGridPosX() - 1 >= 0)
				{
					if (i + creator->getGridPosY() < GAME_WORLD_SIZE_Y && creator->getGridPosX() - 1 < GAME_WORLD_SIZE_X)
					{
						wantedTile = renderer->getTile(creator->getGridPosX() - 1, i + creator->getGridPosY());

						createdUnit = createUnit(wantedTile, isPlayer, optionSelected, menuDisplayed);

						if (createdUnit)
						{
							break;
						}
					}
				}
			}
		}
		//Right
		if (!createdUnit)
		{
			for (int i = -1; i < creator->getGridSizeY() + 1; i++)
			{
				//If in bounds
				if (i + creator->getGridPosY() >= 0 && creator->getGridPosX() + creator->getGridSizeX() >= 0)
				{
					if (i + creator->getGridPosY() < GAME_WORLD_SIZE_Y && creator->getGridPosX() + creator->getGridSizeX() < GAME_WORLD_SIZE_X)
					{
						wantedTile = renderer->getTile(creator->getGridPosX() + creator->getGridSizeX(), i + creator->getGridPosY());

						createdUnit = createUnit(wantedTile, isPlayer, optionSelected, menuDisplayed);

						if (createdUnit)
						{
							break;
						}
					}
				}
			}
		}

		return createdUnit;
	}

	//Function that takes the menu inputs, and game and adds create the proper game object
	GameObject * createUnit(world_tile* wantedTile, const bool player, int optionSelected, int menuDisplayed)
	{
		GameObject * createdUnit = NULL;

		int index = optionSelected + menuDisplayed * MENU_SIZE_X * MENU_SIZE_Y;
		if (index >= unitCreationInfo->size())
		{
			Logger::log_error(std::cout, "Invalid menu / option combination");
			return NULL;
		}
		std::vector<std::string>* unitInfo = textParser.splitString(unitCreationInfo->at(index), '~');

		if (optionSelected >= 0)
		{
			//Make sure its not a tile you can't put stuff on
			if (wantedTile->getCost() < 0)
			{
				return createdUnit;
			}

			createdUnit = renderer->addObjectToScene(player, std::stoi(unitInfo->at(11)), true, unitInfo->at(6), wantedTile->getX(), wantedTile->getY(), std::stoi(unitInfo->at(4)), std::stoi(unitInfo->at(5)), TextureManager::getTextureManager()->getTexture(unitInfo->at(0)), std::stoi(unitInfo->at(1)), std::stoi(unitInfo->at(2)), std::stoi(unitInfo->at(3)), std::stoi(unitInfo->at(7)), std::stoi(unitInfo->at(8)), std::stoi(unitInfo->at(9)), std::stoi(unitInfo->at(10)), unitInfo->at(12));
			
			if (!createdUnit)
				return NULL;

			if(player)
				playerResourceCount -= std::stoi(unitCostInfo->at(optionSelected + menuDisplayed * MENU_SIZE_X * MENU_SIZE_Y));
			else
				opponentResourceCount -= std::stoi(unitCostInfo->at(optionSelected + menuDisplayed * MENU_SIZE_X * MENU_SIZE_Y));
		}
		
		if (player)
			menuOptionSelected = -1;
		return createdUnit;
	}

	void selectObject(GameObject *object)
	{
		// If not already selected, add to vector
		if(std::find(currentMultiSelectedObjects.begin(), currentMultiSelectedObjects.end(), object) == currentMultiSelectedObjects.end())
			currentMultiSelectedObjects.push_back(object);
	}

	void updateKillFeed(std::string newStatment)
	{
		for (int i = 1; i < numberOfKillFeedStrings; i++)
		{
			killFeedStrings[i - 1] = killFeedStrings[i];
		}

		killFeedStrings[numberOfKillFeedStrings - 1] = timeToString(TIME_FOR_GAME, timeStartedGame) + ": " + newStatment;
	}

	//Function to be used by the agent to a viable base
	GameObject* getObjectSpawner(std::string type)
	{
		return renderer->getAgentBase(type);
	}

	GameObject* getGameObject(bool isPlayer, bool isStructure, std::string type, bool isBusy, bool isVisible)
	{
		return renderer->getGameObjectAgent(isPlayer, isStructure, type, isBusy, isVisible);
	}

	void endTheGame(bool playerWon)
	{
		gameIsOver = true;
		paused = true;
		playerWins = playerWon;
	}
};

#endif