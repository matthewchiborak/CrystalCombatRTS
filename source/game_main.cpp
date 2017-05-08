#include "../include/game.h"
#include "../include/pathfinding.h"

TextureManager *TextureManager::s_instance = 0;
PathMap *PathMap::s_instance = 0;

void game_begin(int *status, Lock *lock, Game *game_pointer)
{
	Game &game = *game_pointer;

	sf::RenderWindow window(sf::VideoMode(game.videoModeX, game.videoModeY), "Machine Learning RTS");
	//sf::RenderWindow window(sf::VideoMode(game.videoModeX, game.videoModeY), "Machine Learning RTS", sf::Style::Fullscreen);

	//Read the files to know the game world size and what tiles go where
	std::vector<std::string>* backGroundInfo = game.textParser.parseTextFileByLine("resources/maps/main.map");
	//std::vector<std::string>* backGroundInfo = game.textParser.parseTextFileByLine("resources/text/background.txt");
	//std::vector<std::string>* backGroundInfo = game.textParser.parseTextFileByLine("resources/text/background-big.txt");

	std::vector<std::string>* backGroundInfoSplit;

	game.GAME_WORLD_SIZE_Y = backGroundInfo->size();
	backGroundInfoSplit = game.textParser.splitString(backGroundInfo->at(0), '~');
	game.GAME_WORLD_SIZE_X = backGroundInfoSplit->size();

	//Create other threads. INPUT THREAD MUST BE FIRST SO IT CAN CREATE THE RENDER WINDOW SO THAT IT CAN HANDLE EVENTS
	//std::thread* inputThread = new std::thread(game_input, status, &game, window);

	game.menuFont = new sf::Font();

	//Load the menu font
	if (!game.menuFont->loadFromFile("resources/fonts/ostrich-regular.ttf"))
	{
		*status = -1;
	}

	game.renderer = new Renderer(&game, &window, game.videoModeX / game.TILE_SIZE, game.videoModeY / game.TILE_SIZE, 0, 0, game.TILE_SIZE, game.MIN_ZOOM, game.MAX_ZOOM, game.GAME_WORLD_SIZE_X, game.GAME_WORLD_SIZE_Y, game.menuFont);
	
	//Load The Textures from a text file.
	std::vector<std::string>* textureInfo = game.textParser.parseTextFileByLine("resources/text/textures.txt");
	std::vector<std::string>* texturesSplit;

	for (int i = 0; i < textureInfo->size(); i++)
	{
		texturesSplit = game.textParser.splitString(textureInfo->at(i), '~');
		if (!TextureManager::getTextureManager()->loadTexture(texturesSplit->at(0), "resources/images/" + texturesSplit->at(1)))
		{
			*status = -1;
			return;
		}
	}

	// Texture index
	int index = 0;

	//Testing the background
	for (int j = 0; j < game.GAME_WORLD_SIZE_Y; j++)
	{
		backGroundInfoSplit = game.textParser.splitString(backGroundInfo->at(j), '~');

		//TODO Do a checkif its a differnt number of tiles
		if (backGroundInfoSplit->size() != game.GAME_WORLD_SIZE_X)
		{
			*status = -1;
			return;
		}

		for (int i = 0; i < game.GAME_WORLD_SIZE_X; i++)
		{

			index = stoi(backGroundInfoSplit->at(i));

			//If should generate resources, set the flag to tell it to do so
			if (index == game.PICKUP_GEN_INDEX)
			{
				game.renderer->getTile(i, j)->setResourceGeneration(true);
				game.renderer->addTileToListOfResourceTiles(i, j);
			}

			//game.renderer->giveFogTilesATexture(i, j, TextureManager::getTextureManager()->getTexture("fog"), game.TILE_SIZE);
			game.renderer->giveFogTilesABlankLastSeen(i, j, TextureManager::getTextureManager()->getTexture("blank"), game.TILE_SIZE);
			game.renderer->setBackgroundTilePosition(i, j);
			
			// TEMP COST CODE
			if(index != 0 && index != 2)
				game.renderer->getTile(i, j)->setCost(-1);
		}
	}

	//Read the menu sprite legend
	std::vector<std::string>* menuLegend = game.textParser.parseTextFileByLine("resources/text/menuLegend.txt");
	game.unitCreationInfo = game.textParser.parseTextFileByLine("resources/text/unitCreationInfo.txt");

	//Returns the height of the menu
	float menuHeight = 0;
		
	for (int i = 0; i < menuLegend->size(); i++)
	{
		menuHeight = game.renderer->giveHUDSpriteATexture(&game, TextureManager::getTextureManager()->getTexture(menuLegend->at(i)), game.TILE_SIZE, i, menuLegend->size(), game.UNIT_INFO_BOX_HEIGHT);
	}

	//GIve hud a title screen
	game.renderer->giveTitleScreen(TextureManager::getTextureManager()->getTexture("Title"));

	game.interfaceController.setMenuHeight(menuHeight);

	//The info bar sprite
	game.renderer->setInfoBarSprite(TextureManager::getTextureManager()->getTexture("infoBar"), game.TILE_SIZE, game.UNIT_INFO_BOX_HEIGHT);

	//Give the renderer the selection texture
	game.renderer->setSelectionTexture(&game, TextureManager::getTextureManager()->getTexture("selection"), &game.currentMenuDisplayed);
	game.renderer->setFogOfWarTextureForMasking(TextureManager::getTextureManager()->getTexture("fogTransparent"));
	//game.renderer->giveBigFogATexture(TextureManager::getTextureManager()->getTexture("fog"), game.TILE_SIZE);
	game.renderer->setPickupTexture(TextureManager::getTextureManager()->getTexture("pickup"));

	//Set the background Image
	game.renderer->setBackgroundTexture(TextureManager::getTextureManager()->getTexture("background"), &game);
	
	//Load the unit cost information and give it to the interface controller and the renderer for use for the menu
	game.unitCostInfo = game.textParser.parseTextFileByLine("resources/text/unitCosts.txt");
	game.interfaceController.setUnitCostInfo(game.unitCostInfo);
	game.renderer->giveHUDtheUnitCostInfo(game.unitCostInfo);

	// Initialize the pathfinder map helper
	PathMap::getPathMap()->init(*game.renderer);
	
	// Add player
	game.renderer->addObjectToScene(true, false, false, "test", 7, 9, 1, 1, TextureManager::getTextureManager()->getTexture("ship"), 100, 1, 8, 3, 10, 1, true, "none");
	
	// Add our base
	GameObject* temp = game.renderer->addObjectToScene(true, true, false, "base", 7, 7, 2, 2, TextureManager::getTextureManager()->getTexture("base_good"), 100, 0, 5, 1, 0, 1, false, "worker");
	game.renderer->setMainBase(true, temp);

	// Add enemy 
	game.renderer->addObjectToScene(false, false, false, "test", 31, 22, 1, 1, TextureManager::getTextureManager()->getTexture("wall"), 100, 1, DEFAULT_VISION_RANGE, 1, 3, 1, false, "none");

	// Add enemy base
	GameObject* temp2 = game.renderer->addObjectToScene(false, true, false, "base", 31, 24, 2, 2, TextureManager::getTextureManager()->getTexture("base_bad"), 20, 0, 5, 1, 0, 1, false, "worker");
	game.renderer->setMainBase(false, temp2);

	// Reference to player for pathfinding
	GameObject &player = *game.renderer->getGameObject(true, 0);

	int testPath3[] = { 1,9,1,8,1,7,1,6,1,5,1,4,1,3,1,2,1,1,1,0,1,1,1,2,1,3,1,4,1,5,1,6 };
	//game.renderer->getGameObject(false, 0)->moveWithPath(32, testPath3);

	// Wait for window lock to catch up
	window.setActive(false);
	sleep_milli(15);
	
	//Create the threads -
	std::thread* renderingThread = new std::thread(game_render, status, &game, &window);
	std::thread* logicThread = new std::thread(game_logic, status, &game, lock);

	game.renderer->tellResourceTilesToMakeResources();

	//Fix weird resizing bug
	float newSize = game.renderer->resizeWindow(game.videoModeX, game.videoModeY, &game);
	game.interfaceController.setMenuHeight(newSize);

	// Temporary event queue for polling
	std::queue<sf::Event> tempEventQueue;

	// Game is ready
	game.ready = true;

	while (*status > 0)
	{
		// Events
		while (*status > 0)
		{
			while (!game.windowLock.try_lock());

			window.setActive(true);

			// Event pointer
			sf::Event event;
			while (window.pollEvent(event))
			{
				if (event.type == sf::Event::Closed ||
					(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Q))
				{
					window.close();
					*status = 0;
				}

				//Adjust logic for the resized window
				if (event.type == sf::Event::Resized)
				{
					//
					game.videoModeX = window.getSize().x;
					game.videoModeY = window.getSize().y;
					float newSize = game.renderer->resizeWindow(game.videoModeX, game.videoModeY, &game);
					game.interfaceController.setMenuHeight(newSize);
				}

				if (event.type == sf::Event::GainedFocus)
				{
					game.focus = true;
				}
				if (event.type == sf::Event::LostFocus)
				{
					game.focus = false;
				}

				//Check if player is pausing the game
				if (game.gameHasStarted && !game.gameIsOver && 
					event.type == sf::Event::KeyReleased && 
					(event.key.code == sf::Keyboard::P || 
						event.key.code == sf::Keyboard::Escape))
				{
					if (!game.paused)
					{
						//Not paused
						//Pause the game.

						//Keep track of paused time to add to the start time to prevent timer from counting down while paused
						game.timeOfPause = time(0);
					}
					else
					{
						//Unpause the game
						//See how long the game was paused and add it to the start time
						game.timeStartedGame = game.timeStartedGame + (time(0) - game.timeOfPause);
					}
					
					game.paused = !game.paused;
				}

				//Transition from the title screen 
				if (!game.gameHasStarted && event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Return)
				{
					game.gameHasStarted = true;
					game.paused = false;
					game.timeStartedGame = game.timeStartedGame + (time(0) - game.timeOfPause);
				}

				// Add event to queue
				tempEventQueue.push(event);
			}

			// Push all events to master queue
			if (tempEventQueue.size() > 0)
			{
				while (!game.eventLock.try_lock());

				while (tempEventQueue.size() > 0)
				{
					game.addEvent(tempEventQueue.front());
					tempEventQueue.pop();
				}

				game.eventLock.unlock();
			}

			// Store mouse data
			game.mousePos = sf::Mouse::getPosition(window);

			// Relieve window
			window.setActive(false);

			// Relieve lock
			game.windowLock.unlock();
		}
	}

	// Close app, status is 0 - closing
	*status = 0;

	// Join and clean up threads
	renderingThread->join();
	delete renderingThread;

	logicThread->join();
	delete logicThread;

	//inputThread->join();
	//delete inputThread;
}

void game_logic(int* status, Game* game, Lock* lock)
{
	// Objects to track time
	auto raw_time = std::chrono::system_clock::now().time_since_epoch();
	auto current_time = std::chrono::duration_cast<std::chrono::milliseconds>(raw_time).count();
	auto last_time = current_time;

	// Current event to process
	sf::Event event;

	// Camera positioning
	bool newCam = false;
	int newCamPosX;
	int newCamPosY;

	// Searching for tiles
	bool foundIt;
	bool foundStartTile, foundEndTile;
	int startX, startY, endX, endY;
	int temp;

	// Pathfinding variables
	bool testGetTile;
	world_tile *startTile;
	world_tile *endTile;
	GameObject* objectToAttack;
	GameObject* selectedObject;

	// Pathfinding data structures
	std::vector<int> path;
	PathingList::ListNode *pathIterator = NULL;
	PathingList::ListNode *pathDeleteNode = NULL;
	int* arr = NULL;

	// AI Agent data
	concurrency::concurrent_queue<AgentAction*> &aiActions = game->aiActions;
	AgentAction *currentAction;

	// Loop variables
	int i, j, ix, iy;

	// Multipurpose pointer object
	void *tempPointer = NULL;

	// Main loop
	while (*status > 0)
	{
		//Check the game overstate
		game->renderer->checkGameOver();

		//Game is paused. Don't allow anything
		if (game->paused)
		{
			continue;
		}

		// In order to sync frames, we log the current time
		raw_time = std::chrono::system_clock::now().time_since_epoch();
		current_time = std::chrono::duration_cast<std::chrono::milliseconds>(raw_time).count();

		//Take input and queue it up for processing
		while (!game->eventLock.try_lock());

		while (game->eventCount() > 0)
		{
			// Get next event
			event = game->popEvent();

			//LEFT CLICK - selection of structures/units
			if (game->focus && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				newCam = false;

				//Check if menu is clicked. Returns true if menu was clicked. false if not. Could be used to prevent a move action to square under menu if wanting to click a menu option
				if (!game->interfaceController.determineAction(game->videoModeX, game->videoModeY, game->mousePos.x, game->mousePos.y, &game->menuOptionSelected, game->playerResourceCount, &game->currentMenuDisplayed, game->TRASH_INDEX, game->UNIT_INFO_BOX_HEIGHT, &newCam, &newCamPosX, &newCamPosY, game->GAME_WORLD_SIZE_X, game->GAME_WORLD_SIZE_Y))
				{
					game->menuOptionSelected = -1;
				}

				//If minimapwas clicked, adust teh camera.
				if (newCam)
				{
					game->renderer->centerCamera(newCamPosX, newCamPosY);
				}

				//If unit was selected, try to create it at the base it is at
				if (game->menuOptionSelected >= 0 && game->currentMenuDisplayed != 0)
				{

					int prevMenuDisplayed = game->currentMenuDisplayed;

					GameObject *createdUnit = game->createUnit();

					if (createdUnit)
					{
						//Was it a worker creating the unit? If so give it a build option
						if (prevMenuDisplayed == Menu::worker)
						{
							if (game->singledOutMultiSelectedObjectInd >= 0)
							{
								game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->queueUpBuildAction(createdUnit, game->currentMultiSelectedObjects[0]->getIsPlayer());
							}
							else
							{
								game->currentMultiSelectedObjects[0]->queueUpBuildAction(createdUnit, game->currentMultiSelectedObjects[0]->getIsPlayer());
							}
						}
						//Also for bases
						if (prevMenuDisplayed == Menu::base)
						{
							if (game->singledOutMultiSelectedObjectInd >= 0)
							{
								game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->queueUpBuildAction(createdUnit, game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getIsPlayer());
							}
							else
							{
								game->currentMultiSelectedObjects[0]->queueUpBuildAction(createdUnit, game->currentMultiSelectedObjects[0]->getIsPlayer());
							}
						}
					}

					// Is it a structure?
					if (createdUnit && createdUnit->getStructure())
					{
						//Logger::log(std::cout, "STRUCTURE");
					}
					else
					{
						//Logger::log(std::cout, "NOT STRUCTURE");
					}

					// Returns pointer to new unit
					game->createUnit();

				}

				//Check if trying to delete unit
				if (game->menuOptionSelected == game->TRASH_INDEX)
				{
					//Remove the multiSelectedObjects
					if (game->currentMultiSelectedObjects.size() > 0)
					{
						if (game->singledOutMultiSelectedObjectInd >= 0)
						{
							game->pathList.remove(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]);

							// Remove the unit from the scene
							game->renderer->removeObjectFromScene(game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getID());

							game->currentMultiSelectedObjects.erase(game->currentMultiSelectedObjects.begin() + game->singledOutMultiSelectedObjectInd);
						}
						else
						{
							for (i = 0; i < game->currentMultiSelectedObjects.size(); i++)
							{
								// If this unit was pathfinding, remove it from the pathfinder vector
								game->pathList.remove(game->currentMultiSelectedObjects[i]);

								// Remove the unit from the scene
								game->renderer->removeObjectFromScene(game->currentMultiSelectedObjects.at(i)->getID());
							}

							// No longer selecting any objects
							if (!newCam)
							{
								game->currentMultiSelectedObjects.clear();
							}
						}
						game->menuOptionSelected = -1;
					}
				}

				//Left mouse was clicked, set the start position of the mouse for mouse dragging
				if (!game->mouseDown)
				{
					if (!newCam)
					{
						game->currentMultiSelectedObjects.clear();
						game->singledOutMultiSelectedObjectInd = -1;
					}
					
					game->mouseDown = true;
					game->mouseStartPos = game->mousePos;
				}
			}
			else if (game->mouseDown)
			{
				//Find which objects were hilighted and add them to the vector of selected object
				foundStartTile = false;
				world_tile* startTile = game->renderer->getNearestTileWithMouseCords(&foundStartTile, game->mouseStartPos.x, game->mouseStartPos.y);

				//Need to check if was out of bounds and/or file failed to find

				foundEndTile = false;
				world_tile* endTile = game->renderer->getNearestTileWithMouseCords(&foundEndTile, game->mousePos.x, game->mousePos.y);

				//Need to check if was out of bounds and/or file failed to find
				if (foundStartTile && foundEndTile)
				{
					//Rearrange x and y values so always iterate up
					startX = startTile->getX();
					startY = startTile->getY();
					endX = endTile->getX();
					endY = endTile->getY();

					if (endX < startX)
					{
						temp = endX;
						endX = startX;
						startX = temp;
					}
					if (endY < startY)
					{
						temp = endY;
						endY = startY;
						startY = temp;
					}

					//Iterate overall all the tiles in the box, check if an object is on it, and if so, select it
					for (ix = startX; ix <= endX; ix++)
					{
						for (iy = startY; iy <= endY; iy++)
						{
							world_tile* focusTile = game->renderer->getTile(ix, iy);

							if (focusTile->getOccupiedObj())
							{
								//There's an object on it. Make sure that it is a player game object and if so select it
								//GameObject* focusObject = focusTile->getOccupiedObj();
								
								//if (focusObject)
								for(int multInd = 0; multInd < focusTile->numberOfObjectsOnMe(); multInd++)
								{
									GameObject* focusObject = focusTile->getObjectOnMeWithIndex(multInd);

									//Check if its a player object
									if (focusObject->getIsPlayer())
									{
										//Add the object to the multiselection
										game->selectObject(focusObject);
									}
								}
							}
						}
					}
				}

				//Mouse was released. Reset the drag box
				game->mouseStartPos = game->mousePos;
				game->mouseDown = false;
			}
			else
			{
				//Mouse was released. Reset the drag box
				game->mouseStartPos = game->mousePos;
				game->mouseDown = false;
			}

			// Hide any existing menu
			game->currentMenuDisplayed = 0;

			// Do we display a menu?
			if (game->currentMultiSelectedObjects.size() == 1)
			{
				// If there is only one object selected and it is a structure, show its menu
				tempPointer = (void *)game->currentMultiSelectedObjects[0];
				if (((GameObject*)tempPointer)->getStructure())
				{
					// TODO remove hardcoded values and have enum mappings
					if (((GameObject*)tempPointer)->getType() == "base")
					{
						if (!((GameObject*)tempPointer)->getBeingBuilt())
						{
							game->currentMenuDisplayed = Menu::base;
						}
					}
				}
				else
				{
					if (((GameObject*)tempPointer)->getType() == "worker")
					{
						if (!((GameObject*)tempPointer)->getBeingBuilt())
						{
							game->currentMenuDisplayed = Menu::worker;
						}
					}
				}
			}
			else if (game->currentMultiSelectedObjects.size() > 0 && game->singledOutMultiSelectedObjectInd >= 0)
			{
				// If there is only one object selected and it is a structure, show its menu
				tempPointer = (void *)game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd];
				if (((GameObject*)tempPointer)->getStructure())
				{
					// TODO remove hardcoded values and have enum mappings
					if (((GameObject*)tempPointer)->getType() == "base")
					{
						if (!((GameObject*)tempPointer)->getBeingBuilt())
						{
							game->currentMenuDisplayed = Menu::base;
						}
					}
				}
				else
				{
					if (((GameObject*)tempPointer)->getType() == "worker")
					{
						if (!((GameObject*)tempPointer)->getBeingBuilt())
						{
							game->currentMenuDisplayed = Menu::worker;
						}
					}
				}
			}

			//Pressing Left and Right on the Keyboard to select one object if multiple objects are selected
			if (!game->arrowPressed && game->focus && (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)))
			{
				game->arrowPressed = true;

				if (game->singledOutMultiSelectedObjectInd < 0)
				{
					game->singledOutMultiSelectedObjectInd = 0;
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
				{
					game->singledOutMultiSelectedObjectInd--;

					if (game->singledOutMultiSelectedObjectInd < 0)
					{
						game->singledOutMultiSelectedObjectInd = game->currentMultiSelectedObjects.size() - 1;
					}
				}
				else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
				{
					game->singledOutMultiSelectedObjectInd++;

					if (game->singledOutMultiSelectedObjectInd >= game->currentMultiSelectedObjects.size())
					{
						game->singledOutMultiSelectedObjectInd = 0;
					}
				}
			}
			else if(game->focus && !sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && !sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
			{
				game->arrowPressed = false;
			}

			//RIGHT CLICK - actions
			if (game->focus && sf::Mouse::isButtonPressed(sf::Mouse::Right))
			{

				//Reset for move
				game->selectedAction = Action::Move;

				if (game->currentMultiSelectedObjects.size() == 1)
				{
					//Build Actions
					//Also if a friendly object is at the destination and it is a worker unit, queue a build action as well
					endTile = game->renderer->getTileWithMouseCords(&testGetTile, game->mousePos.x, game->mousePos.y);
					if (endTile)
					{
						if (endTile->getOccupiedObj())
						{
							//If it cant move cancel the moveaction then give it the build action. If it can move just queue it
							if (game->currentMultiSelectedObjects.size() == 1)
							{
								selectedObject = game->currentMultiSelectedObjects[0];
								if (endTile->getOccupiedObj()->getBeingBuilt() && ((selectedObject->getIsPlayer() && endTile->getOccupiedObj()->getIsPlayer()) || (!selectedObject->getIsPlayer() && !endTile->getOccupiedObj()->getIsPlayer())))
								{
									selectedObject->queueUpBuildAction(endTile->getOccupiedObj(), selectedObject->getIsPlayer());
									game->selectedAction = Action::Build;
								}
							}
						}
					}
				}
				else if (game->currentMultiSelectedObjects.size() > 0 && game->singledOutMultiSelectedObjectInd >= 0)
				{
					//Build Actions
					//Also if a friendly object is at the destination and it is a worker unit, queue a build action as well
					endTile = game->renderer->getTileWithMouseCords(&testGetTile, game->mousePos.x, game->mousePos.y);
					if (endTile)
					{
						if (endTile->getOccupiedObj())
						{
							//If it cant move cancel the moveaction then give it the build action. If it can move just queue it
							//if (game->currentMultiSelectedObjects.size() == 1)
							{
								selectedObject = game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd];
								if ((selectedObject->getIsPlayer() && endTile->getOccupiedObj()->getIsPlayer()) || (!selectedObject->getIsPlayer() && !endTile->getOccupiedObj()->getIsPlayer()))
								{
									selectedObject->queueUpBuildAction(endTile->getOccupiedObj(), selectedObject->getIsPlayer());
									game->selectedAction = Action::Build;
								}
							}
						}
					}
				}


				if (game->currentMultiSelectedObjects.size() > 0)
				{
					if (!game->actionSelected)
					{
						game->actionSelected = true;
						game->selectedAction = Action::Move;
					}

					if (game->actionSelected)
					{
						// Perform action
						switch (game->selectedAction)
						{
						case(Action::Build): break;
						case(Action::Move):
						{
							// Ensure that user clicked on a valid tile
							endTile = game->renderer->getTileWithMouseCords(&testGetTile, game->mousePos.x, game->mousePos.y);
							if (endTile)
							{
								if (game->singledOutMultiSelectedObjectInd < 0)
								{
									//Loop through all multi selected objects and, if not structures, move them
									for (i = 0; i < game->currentMultiSelectedObjects.size(); i++)
									{
										// Only non-structures can move
										if (!game->currentMultiSelectedObjects[i]->getStructure())
										{
											// Start location
											startTile =
												game->renderer->getTile(
													game->currentMultiSelectedObjects[i]->getGridPosX(),
													game->currentMultiSelectedObjects[i]->getGridPosY());

											// There is a start location
											if (startTile)
											{
												//If there's an enemy on the target location, make it a target of the object
												if (endTile->getOccupiedObj())
												{
													objectToAttack = endTile->getOccupiedObj();

													if (objectToAttack)
													{
														if (!objectToAttack->getIsPlayer())
														{
															game->currentMultiSelectedObjects[i]->setAttackTarget(objectToAttack);
														}
													}
												}
												else
												{
													game->currentMultiSelectedObjects[i]->setAttackTarget(NULL);
												}

												// Initiate pathfind algorithm
												Game::moveToLocation(game->currentMultiSelectedObjects[i], game->pathList, startTile, endTile);
											}
										}
									}//This is the for loop
								}
								else if(game->singledOutMultiSelectedObjectInd >= 0)
								{
									// Only non-structures can move
									if (!game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getStructure())
									{
										// Start location
										startTile =
											game->renderer->getTile(
												game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getGridPosX(),
												game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getGridPosY());

										// There is a start location
										if (startTile)
										{
											//If there's an enemy on the target location, make it a target of the object
											if (endTile->getOccupiedObj())
											{
												objectToAttack = endTile->getOccupiedObj();

												if (objectToAttack)
												{
													if (!objectToAttack->getIsPlayer())
													{
														game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->setAttackTarget(objectToAttack);
													}
												}
											}
											else
											{
												game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->setAttackTarget(NULL);
											}

											// Initiate pathfind algorithm
											Game::moveToLocation(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd],
												game->pathList, startTile, endTile);
										}
									}
								}
							}
						}
						}
					}
				}
				else
				{
					game->actionSelected = false;
					game->selectedAction = Action::None;
					if (!newCam)
					{
						game->currentMultiSelectedObjects.clear();
						game->singledOutMultiSelectedObjectInd = -1;
					}
				}
			}

			//Zoom using the mouse wheel
			if (game->focus && event.type == sf::Event::MouseWheelMoved)
			{
				game->renderer->zoomView(event.mouseWheel.delta * -game->ZOOM_SPEED);
			}
		}

		//Move the camera by moving the mouse to the edge of the screen
		if (game->focus)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || game->mousePos.x < game->SCREEN_EDGE_SCROLL_LENGTH && game->mousePos.y < (game->videoModeY) - game->renderer->getMenuHeight() - game->UNIT_INFO_BOX_HEIGHT)
			{
				game->renderer->moveCamera(-1 * game->CAMERA_MOVE_SPEED, 0);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || game->mousePos.x > game->videoModeX - game->SCREEN_EDGE_SCROLL_LENGTH && game->mousePos.y < (game->videoModeY) - game->renderer->getMenuHeight() - game->UNIT_INFO_BOX_HEIGHT)
			{
				game->renderer->moveCamera(game->CAMERA_MOVE_SPEED, 0);
			}
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || game->mousePos.y < game->SCREEN_EDGE_SCROLL_LENGTH)
			{
				game->renderer->moveCamera(0, -1 * game->CAMERA_MOVE_SPEED);
			}

			if (game->currentMenuDisplayed != 0)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || game->mousePos.y > (game->videoModeY - game->SCREEN_EDGE_SCROLL_LENGTH) - game->renderer->getMenuHeight() - game->UNIT_INFO_BOX_HEIGHT && game->mousePos.y < (game->videoModeY) - game->renderer->getMenuHeight() - game->UNIT_INFO_BOX_HEIGHT)
				{
					game->renderer->moveCamera(0, game->CAMERA_MOVE_SPEED);
				}
			}
			else
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || game->mousePos.y >(game->videoModeY - game->SCREEN_EDGE_SCROLL_LENGTH - game->UNIT_INFO_BOX_HEIGHT) && game->mousePos.y < (game->videoModeY - game->UNIT_INFO_BOX_HEIGHT))
				{
					game->renderer->moveCamera(0, game->CAMERA_MOVE_SPEED);
				}
			}
		}

		// Relieve lock
		game->eventLock.unlock();

		// Pathfind update
		pathIterator = game->pathList.head();
		while(pathIterator)
		{
			selectedObject = pathIterator->object;
			if(selectedObject && selectedObject->pathLock.try_lock())
			{
				if (selectedObject->pathfinder)
				{
					// Pathfinding is done or never started
					if (selectedObject->pathfinder->finished() &&
						!selectedObject->pathfinder->cancelled())
					{
						// Reset vector of path tiles
						path.clear();

						// Start at end and work back to front
						Tile *node = selectedObject->pathfinder->endPos();

						// Path found - reconstruct it by following parent references
						while (node && node->getParent())
						{
							path.insert(path.begin(), node->getY());
							path.insert(path.begin(), node->getX());
							node = node->getParent();
						}

						// Set up the array to hold the path
						arr = new int[path.size()];

						// Copy path into the array
						std::copy(path.begin(), path.end(), arr);

						// Give instructions to the unit
						selectedObject->moveWithPath(path.size(), arr);

						// Clean up arr for next time
						delete arr;
						arr = NULL;
					}

					// Done pathfinding, clean up the threads and pathfinder
					if (selectedObject->pathfinder->cancelled() ||
						selectedObject->pathfinder->finished())
					{
						// No longer need pathfinder - clear object
						selectedObject->pathLock.unlock();
						delete selectedObject->pathfinder;
						// Waits for the pathfinder to finish deleting
						while (!selectedObject->pathLock.try_lock());
						selectedObject->pathfinder = NULL;
					}

					// Done pathfinding, remove from the pathfinder
					pathDeleteNode = pathIterator;
				}

				// Return the mutex
				selectedObject->pathLock.unlock();
			}

			pathIterator = pathIterator->next;

			if (pathDeleteNode)
			{
				game->pathList.remove(pathDeleteNode->object);
				pathDeleteNode = NULL;
			}
		}

		// AI actions
		if (!aiActions.empty())
		{
			// Process next action
			game->aiProcessing = true;
			if (aiActions.try_pop(currentAction))
			{
				// All actions have a main object to initiate the action
				for (int i = 0; i < currentAction->ids.size(); i++)
				{
					GameObject *current = game->renderer->getGameObjectWithId(false, currentAction->ids[i]);

					if (current)
					{
						// Each type handled differently
						switch (currentAction->type)
						{
						case(Action::Attack) :
						{
							// Target to attack
							if (currentAction->target >= 0)
							{
								GameObject *target = game->renderer->getGameObjectWithId(true, currentAction->target);

								if (target &&
									current->getAttackTarget() != target)
								{
									// If valid target that is not already targetted
									current->setAttackTarget(target);

									// Need to not be busy and prioritize the attack
									if (current->busy())
									{
										// As long as it is not being built, we free the unit
										if (!current->getBeingBuilt())
										{
											// Cancel pathfinding thread
											if (current->pathfinder)
												current->pathfinder->cancel();

											// Cancel all actions
											current->cancelActions();
										}
									}
								}

								// If not in range - or invalid target - move to target
								if (!attackTargetInRange(current, target))
								{
									if (target)
									{
										Game::moveToLocation(
											current,
											game->pathList,
											game->renderer->getTile(current->getGridPosX(), current->getGridPosY()),
											game->renderer->getTile(target->getGridPosX(), target->getGridPosY()));
									}
								}
							}
							break;
						}
						case(Action::Build) :
						{
							GameObject *createdUnit = game->createUnit(current, false, currentAction->buildOption, currentAction->buildMenu);
							if (createdUnit)
							{
								current->queueUpBuildAction(createdUnit, false);
								if (createdUnit->getStructure())
									game->actualOpponent.structures++;
								else
									game->actualOpponent.units++;
							}
							break;
						}
						case(Action::Delete) :
						{
							GameObject *target = game->renderer->getGameObjectWithId(true, currentAction->target);
							if (target)
							{
								if (target->getStructure())
									game->actualOpponent.structures--;
								else
									game->actualOpponent.units--;
								target->disable();
							}
							break;
						}
						case(Action::Farm) :
						case(Action::Move) :
						case(Action::Scout) :
						{
							// If free it moves
							if (!current->busy())
							{
								Game::moveToLocation(current, 
									game->pathList,
									game->renderer->getTile(current->getGridPosX(), current->getGridPosY()),
									game->renderer->getTile(currentAction->targetTile.x, currentAction->targetTile.y));
							}
							break;
						}
						case(Action::Idle) :
						case(Action::None) :
						default:
							// No action - remove from queue
							break;
						}
					}
				}

				// Done processing this action
				game->aiProcessing = false;
				delete currentAction;
			}
		}

		// Compare
		if (current_time - last_time >= game->FRAME_TIME)
		{
			// Store last time
			last_time = current_time;
			//Tell the game objects to execute actions
			if (game->focus)
			{
				game->renderer->tellObjectsToDoActions(game);
				game->renderer->giveBasesUnitResources(&game->playerResourceCount, &game->opponentResourceCount);
			}
		}
	}
}

void game_render(int* status, Game* game, sf::RenderWindow* window)
{
	while (*status > 0 && window->isOpen())
	{
		while (!game->windowLock.try_lock());

		window->setActive(true);
		window->clear();
 
		game->renderer->drawScene(game); 

		window->display();

		// Relieve lock
		window->setActive(false);
		game->windowLock.unlock();
	}

	if (window->isOpen())
		window->close();

	*status = 0;
}