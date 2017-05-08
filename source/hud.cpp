#include "../include/hud.h"
#include "../include/game.h"
#include "../include/world_tile.h"
#include "../include/fog.h"


hud::hud()
{
	HUDSprite = NULL;
}


hud::~hud()
{
}

void hud::setSelectionTexture(Game* game, sf::Texture* textureToSet, int windowSizeX, float squareSize, int menuSizeX, int menuSizeY, int* currentlySelectedMenu)
{
	currentlyDisplayedMenu = currentlySelectedMenu;

	menuSelector.setTexture(*textureToSet);
	float scaleFactorX = ((game->videoModeX) / menuSizeX) / (float)textureToSet->getSize().x;
	float scaleFactorY = (((float)menuHeight) / menuSizeY) / (float)textureToSet->getSize().y;

	menuSelector.setScale(sf::Vector2f(scaleFactorX, scaleFactorY));

	menuSelector2.setTexture(*textureToSet);
	menuSelector2.setScale(sf::Vector2f(scaleFactorX, scaleFactorY));
}

void hud::drawHUD(Game* game, sf::RenderWindow* gameWindow, int windowSizeX, int windowSizeY, int menuSizeX, int menuSizeY, world_tile** backgroundTiles, Fog** fogOfWar, std::vector<GameObject*>* playerInGameObjects, std::vector<GameObject*>* opponentInGameObjects, int windowPosX, int windowPosY, float zoom, float bx, float cx, float by, float cy)
{
	//Draw the selection box from mouse drag
	if (!game->paused && game->mouseDown)
	{
		sf::RectangleShape rectangleSelect;
		rectangleSelect.setSize(sf::Vector2f(game->mousePos.x - game->mouseStartPos.x, game->mousePos.y - game->mouseStartPos.y));
		rectangleSelect.setFillColor(sf::Color(1, 1, 1, 156));
		rectangleSelect.setPosition(game->mouseStartPos.x, game->mouseStartPos.y);
		gameWindow->draw(rectangleSelect);
	}

	//Draw the info bar first so that text will go on top of it
	gameWindow->draw(infoBarSprite); 

	//Find the resized menuSquareSize
	float menuSquareSize = game->videoModeX / menuSizeX;

	//Only display if structure is selected
	if (*currentlyDisplayedMenu != 0)
	{
		//Draw the background
		for (int i = 0; i < menuSizeX * menuSizeY; i++)
		{
			if (*currentlyDisplayedMenu * menuSizeX * menuSizeY + i < totalIcons)
			{
				if (i % menuSizeX != 0 && i % menuSizeX != 1 && i % menuSizeX != 2)
				{
					gameWindow->draw(HUDSprite[*currentlyDisplayedMenu * menuSizeX * menuSizeY + i]);
				}
			}
		}
	}


	//Draw the minimap
	gameWindow->draw(minimap);

	float miniMapTileSize = (float)menuHeight / (float)game->GAME_WORLD_SIZE_Y;
	sf::RectangleShape mapIcon;
	mapIcon.setSize(sf::Vector2f(miniMapTileSize, miniMapTileSize));
	int xCo = 0;
	int yCo = 0;

	//Draw all the player objects on the minimap
	for (int pId = 0; pId < playerInGameObjects->size(); pId++)
	{
		if (playerInGameObjects->at(pId)->getActive())
		{
			for (int wid = 0; wid < playerInGameObjects->at(pId)->getGridSizeX(); wid++)
			{
				for (int hei = 0; hei < playerInGameObjects->at(pId)->getGridSizeY(); hei++)
				{
					xCo = playerInGameObjects->at(pId)->getGridPosX() + wid;
					yCo = playerInGameObjects->at(pId)->getGridPosY() + hei;

					mapIcon.setPosition(xCo * miniMapTileSize, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight + (yCo * miniMapTileSize));

					if (playerInGameObjects->at(pId)->getStructure())
					{
						mapIcon.setFillColor(sf::Color::Blue);
					}
					else
					{
						mapIcon.setFillColor(sf::Color::Green);
					}

					gameWindow->draw(mapIcon);
				}
			}
		}

		//Draw of opponent objects but they must be in view
		for (int pId = 0; pId < opponentInGameObjects->size(); pId++)
		{
			if (opponentInGameObjects->at(pId)->getActive())
			{
				for (int wid = 0; wid < opponentInGameObjects->at(pId)->getGridSizeX(); wid++)
				{
					for (int hei = 0; hei < opponentInGameObjects->at(pId)->getGridSizeY(); hei++)
					{
						xCo = opponentInGameObjects->at(pId)->getGridPosX() + wid;
						yCo = opponentInGameObjects->at(pId)->getGridPosY() + hei;

						if (!fogOfWar[xCo][yCo].getEnabled() && fogOfWar[xCo][yCo].getCurrentVisiblePlayer())
						{
							mapIcon.setPosition(xCo * miniMapTileSize, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight + (yCo * miniMapTileSize));

							mapIcon.setFillColor(sf::Color::Red);

							gameWindow->draw(mapIcon);
						}
					}
				}
			}
		}
			

		//Draw the camera position
		sf::RectangleShape cameraIcon;
		cameraIcon.setSize(sf::Vector2f(windowSizeX * miniMapTileSize * zoom, windowSizeY * miniMapTileSize * zoom));
		cameraIcon.setFillColor(sf::Color::Transparent);
		cameraIcon.setOutlineColor(sf::Color::Red);
		cameraIcon.setOutlineThickness(1);
		int minVisibileX = windowPosX + bx * zoom + cx + ((0) / (game->TILE_SIZE / zoom));
		int minVisibileY = windowPosY + by * zoom + cy + ((0) / (game->TILE_SIZE / zoom));
		cameraIcon.setPosition((minVisibileX) * miniMapTileSize, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight + (miniMapTileSize * (minVisibileY)));

		//Do some masking
		sf::RenderTexture visionHole;
		visionHole.create(game->videoModeX, game->videoModeY);
		visionHole.clear(sf::Color::Transparent);
		visionHole.draw(cameraIcon);

		//THe four edges to cutout
		sf::RectangleShape cameraCutout1;
		sf::RectangleShape cameraCutout2;
		sf::RectangleShape cameraCutout3;

		//Change sizes and positions and colour
		cameraCutout1.setFillColor(sf::Color::Magenta);
		cameraCutout2.setFillColor(sf::Color::Magenta);
		cameraCutout3.setFillColor(sf::Color::Magenta);
	
		cameraCutout1.setSize(sf::Vector2f(game->videoModeX, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight));
		cameraCutout2.setSize(sf::Vector2f(game->videoModeX, game->videoModeY));
		cameraCutout3.setSize(sf::Vector2f(game->videoModeX, game->videoModeY));

		cameraCutout2.setPosition(game->GAME_WORLD_SIZE_X * miniMapTileSize, 0);
		cameraCutout3.setPosition(0, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT);

		visionHole.draw(cameraCutout1);
		visionHole.draw(cameraCutout2);
		visionHole.draw(cameraCutout3);

		visionHole.display();

		sf::Image holeImage;
		holeImage = visionHole.getTexture().copyToImage();
		holeImage.createMaskFromColor(sf::Color::Magenta);

		sf::Texture tex;
		tex.loadFromImage(holeImage);

		gameWindow->draw(sf::Sprite(tex));


	}

	//If no menu draw trash
	if (*currentlyDisplayedMenu == 0)
	{
		gameWindow->draw(HUDSprite[menuSizeX * menuSizeY - 1]);
	}

	
	resourceCount.setString(std::to_string(game->playerResourceCount));

	resourceCount.setPosition(0, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT);

	if (game->currentMultiSelectedObjects.size() > 0)
	{
		if (game->singledOutMultiSelectedObjectInd >= 0)
		{
			unitInfoDisplay.setString((game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getType()) + " " + std::to_string(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getID())
				+ " HP: " + floatToString(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getCurrentHealth(), 1)
				+ "/" + floatToString(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getMaxHealth(), 1)
				+ " ATT: " + floatToString(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getAttackPower(), 1)
				+ " RNG: " + floatToString(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getAttackRange(), 1)
				+ " RATE: " + floatToString(game->currentMultiSelectedObjects[game->singledOutMultiSelectedObjectInd]->getAttackSpeed(), 1)
				+ " SPEED: " + floatToString(game->currentMultiSelectedObjects[0]->getSpeed(), 1));
		}
		else if (game->currentMultiSelectedObjects.size() == 1)
		{
			unitInfoDisplay.setString((game->currentMultiSelectedObjects[0]->getType()) + " " + std::to_string(game->currentMultiSelectedObjects[0]->getID())
				+ " HP: " + floatToString(game->currentMultiSelectedObjects[0]->getCurrentHealth(), 1)
				+ "/" + floatToString(game->currentMultiSelectedObjects[0]->getMaxHealth(), 1)
				+ " ATT: " + floatToString(game->currentMultiSelectedObjects[0]->getAttackPower(), 1)
				+ " RNG: " + floatToString(game->currentMultiSelectedObjects[0]->getAttackRange(), 1)
				+ " RATE: " + floatToString(game->currentMultiSelectedObjects[0]->getAttackSpeed(), 1)
				+ " SPEED: " + floatToString(game->currentMultiSelectedObjects[0]->getSpeed(), 1));
		}
		else
		{
			std::string objectIDsHere = "Units: ";

			for (int idInd = 0; idInd < game->currentMultiSelectedObjects.size(); idInd++)
			{
				objectIDsHere = objectIDsHere + std::to_string(game->currentMultiSelectedObjects[idInd]->getID()) + ", ";
			}

			//unitInfoDisplay.setString(("ID: " + std::to_string(game->currentMultiSelectedObjects[0]->getID())));
			unitInfoDisplay.setString(objectIDsHere);
		}
	}
	else
	{
		unitInfoDisplay.setString((" "));
	}
	
	
	unitInfoDisplay.setPosition(100, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT);

	// Draw it
	gameWindow->draw(resourceCount);
	gameWindow->draw(unitInfoDisplay);

	//Only update the timer if the game is not paused
	if (!game->paused)
	{
		//Draw the in game time past
		std::string timeToDisplay = std::to_string(((game->TIME_FOR_GAME - (time(0) - game->timeStartedGame)) / 60)) + ":";

		if (((game->TIME_FOR_GAME - (time(0) - game->timeStartedGame)) % 60) < 10)
		{
			timeToDisplay = timeToDisplay + "0" + std::to_string(((game->TIME_FOR_GAME - (time(0) - game->timeStartedGame)) % 60));
		}
		else
		{
			timeToDisplay = timeToDisplay + std::to_string(((game->TIME_FOR_GAME - (time(0) - game->timeStartedGame)) % 60));
		}


		timeText.setString(timeToDisplay);
	}

	gameWindow->draw(timeText);

	//Draw the killfeed
	killFeedText.setPosition(0, game->videoModeY - menuHeight - game->UNIT_INFO_BOX_HEIGHT);
	std::string newKillFeed = "";

	for (int f = 0; f < game->numberOfKillFeedStrings; f++)
	{
		newKillFeed = newKillFeed + game->killFeedStrings[f] + "\n";
	}

	killFeedText.setString(newKillFeed);
	killFeedText.setOrigin(0, killFeedText.getLocalBounds().height);
	gameWindow->draw(killFeedText);

	//If a single object is selected, display a picture of it as an icon
	if (game->singledOutMultiSelectedObjectInd >= 0)
	{
		sf::Sprite infoBarIcon;
		infoBarIcon.setTexture(*game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getSprite().getTexture());
		infoBarIcon.setScale(game->UNIT_INFO_BOX_HEIGHT / game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getSprite().getTexture()->getSize().y, game->UNIT_INFO_BOX_HEIGHT / game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getSprite().getTexture()->getSize().y);
		infoBarIcon.setPosition(50, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT);

		gameWindow->draw(infoBarIcon);
	}
	else if (game->currentMultiSelectedObjects.size() == 1)
	{
		sf::Sprite infoBarIcon;
		infoBarIcon.setTexture(*game->currentMultiSelectedObjects.at(0)->getSprite().getTexture());
		
		infoBarIcon.setScale(game->UNIT_INFO_BOX_HEIGHT / game->currentMultiSelectedObjects.at(0)->getSprite().getTexture()->getSize().y, game->UNIT_INFO_BOX_HEIGHT / game->currentMultiSelectedObjects.at(0)->getSprite().getTexture()->getSize().y);
		infoBarIcon.setPosition(50, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT);

		gameWindow->draw(infoBarIcon);
	}

	if (*currentlyDisplayedMenu != 0)
	{

		//If the mouse is over an icon, put up the selection id for feedback
		if (game->mousePos.y > ((game->videoModeY) - menuHeight - game->UNIT_INFO_BOX_HEIGHT) && game->mousePos.y < (game->videoModeY - game->UNIT_INFO_BOX_HEIGHT) && game->mousePos.x > 0 && game->mousePos.x < game->videoModeX)
		{
			int boxX = game->mousePos.x / ((game->videoModeX) / menuSizeX);
			int boxY = (game->mousePos.y - (game->videoModeY - menuHeight - game->UNIT_INFO_BOX_HEIGHT)) / ((menuHeight) / menuSizeY);

			//Draw the cost of that unit. If enough funds text should be green. If not. text will be red
			if (boxX % menuSizeX != 0 && boxX % menuSizeX != 1 && boxX % menuSizeX != 2)
			{
				costText.setString("$" + unitCostInfo->at((boxY * menuSizeX + boxX) + (*currentlyDisplayedMenu * menuSizeX * menuSizeY)));

				costText.setPosition(boxX * ((game->videoModeX) / menuSizeX) + 0.25 * ((game->videoModeX) / menuSizeX), game->videoModeY - menuHeight - game->UNIT_INFO_BOX_HEIGHT + boxY * (menuHeight / menuSizeY));


				if (game->playerResourceCount >= std::stoi(unitCostInfo->at((boxY * menuSizeX + boxX) + (*currentlyDisplayedMenu * menuSizeX * menuSizeY))))
				{
					costText.setColor(sf::Color::Green);
				}
				else
				{
					costText.setColor(sf::Color::Red);
				}

				//Don't render the cost if its a free action
				if (std::stoi(unitCostInfo->at((boxY * menuSizeX + boxX) + (*currentlyDisplayedMenu * menuSizeX * menuSizeY))) > 0)
				{
					gameWindow->draw(costText);
				}

				menuSelector.setPosition(boxX * ((game->videoModeX) / menuSizeX), game->videoModeY - menuHeight - game->UNIT_INFO_BOX_HEIGHT + boxY * (menuHeight / menuSizeY));
				gameWindow->draw(menuSelector);
			}
		}

		//Keep the selected icon if selected a mneu option
		//Also currently selected box
		if (game->currentMenuDisplayed >= 0)
		{
			int boxX = (game->currentMenuDisplayed % menuSizeX);
			int boxY = (game->currentMenuDisplayed / menuSizeX);

			menuSelector2.setPosition(boxX * ((game->videoModeX) / menuSizeX), game->videoModeY - menuHeight - game->UNIT_INFO_BOX_HEIGHT + boxY * (menuHeight / menuSizeY));
			
		}
	}

	if (game->paused)
	{
		if (game->gameIsOver)
		{
			if (game->playerWins)
			{
				pausedText.setString("You Win!");
			}
			else
			{
				pausedText.setString("You Lose...");
			}
		}

		pausedText.setOrigin(pausedText.getLocalBounds().width / 2, pausedText.getLocalBounds().height);
		pausedText.setPosition(game->videoModeX / 2, game->videoModeY / 2);

		gameWindow->draw(pausedText);
	}

	if (!game->gameHasStarted)
	{
		gameWindow->draw(titleScreen);

		titleScreenText.setOrigin(titleScreenText.getLocalBounds().width / 2, titleScreenText.getLocalBounds().height);
		titleScreenText.setPosition(game->videoModeX / 2, game->videoModeY / 2);
		gameWindow->draw(titleScreenText);
	}
}

float hud::giveHUDSpriteATexture(Game* game, sf::Texture* textToSet, float squareSize, int windowSizeX, int windowSizeY, int menuSizeX, int menuSizeY, int index, int totalIcons, float infoHeight)
{
	this->totalIcons = totalIcons;

	if (!HUDSprite)
	{
		HUDSprite = new sf::Sprite[totalIcons];
	}

	
	float scaleFactor = ((game->videoModeX) / (float)textToSet->getSize().x) / menuSizeX;

	HUDSprite[index].setTexture(*textToSet);
	HUDSprite[index].setScale(sf::Vector2f(scaleFactor, scaleFactor));
	HUDSprite[index].setOrigin(0, textToSet->getSize().y);
	HUDSprite[index].setPosition(((index % (menuSizeX * menuSizeY)) % menuSizeX) * ((game->videoModeX) / menuSizeX), (((index % (menuSizeX * menuSizeY)) / menuSizeX) * ((game->videoModeX) / menuSizeX)) + ((game->videoModeY) - (textToSet->getSize().y * scaleFactor)) - infoHeight);

	menuHeight = textToSet->getSize().y * scaleFactor * menuSizeY;

	return textToSet->getSize().y * scaleFactor * menuSizeY;
}

float hud::resizeHUD(Game *game, int windowSizeX, int windowSizeY, float squareSize, int menuSizeX, int menuSizeY, float x, float y, int playerResourceCount, int currentMenuSelected, GameObject* selectedObject, float infoHeight)
{
	float scaleFactor = ((game->videoModeX) / (float)HUDSprite[0].getTexture()->getSize().x) / menuSizeX;
	menuHeight = HUDSprite[0].getTexture()->getSize().y * scaleFactor * menuSizeY;

	for (int i = 0; i < totalIcons; i++)
	{
		//Set the position and size based on the windowSize
	
		HUDSprite[i].setScale(sf::Vector2f(scaleFactor, scaleFactor));
		
		HUDSprite[i].setPosition(((i % (menuSizeX * menuSizeY)) % menuSizeX) * ((game->videoModeX) / menuSizeX), (((i % (menuSizeX * menuSizeY)) / menuSizeX) * ((game->videoModeX / menuSizeX)) + ((game->videoModeY) - (HUDSprite[i].getTexture()->getSize().y * scaleFactor)) - infoHeight));
	}

	infoBarSprite.setScale(game->videoModeX / infoBarSprite.getTexture()->getSize().x, infoHeight / infoBarSprite.getTexture()->getSize().y);
	infoBarSprite.setPosition(0, game->videoModeY - infoHeight);

	

	
	float scaleFactorX = (((float)game->videoModeX) / menuSizeX) / (float)menuSelector.getTexture()->getSize().x;
	float scaleFactorY = (((float)menuHeight) / menuSizeY) / (float)menuSelector.getTexture()->getSize().y;

	menuSelector.setScale(sf::Vector2f(scaleFactorX, scaleFactorY));
	menuSelector2.setScale(sf::Vector2f(scaleFactorX, scaleFactorY));

	minimap.setScale((((float)menuHeight / (float)game->GAME_WORLD_SIZE_Y) * (float)game->GAME_WORLD_SIZE_X) / minimap.getTexture()->getSize().x, menuHeight / minimap.getTexture()->getSize().y);
	minimap.setPosition(0, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight);

	titleScreen.setScale(game->videoModeX / titleScreen.getTexture()->getSize().x, game->videoModeY / titleScreen.getTexture()->getSize().y);
	
	return menuHeight;
}


void hud::assignFont(sf::Font* menuFont)
{
	this->menuFont = menuFont;
	resourceCount.setFont(*menuFont);
	resourceCount.setCharacterSize(30);
	resourceCount.setColor(sf::Color::White);

	unitInfoDisplay.setFont(*menuFont);
	unitInfoDisplay.setCharacterSize(30);
	unitInfoDisplay.setColor(sf::Color::White);

	costText.setFont(*menuFont);
	costText.setCharacterSize(20);

	timeText.setFont(*menuFont);
	timeText.setCharacterSize(30);
	timeText.setColor(sf::Color::White);

	killFeedText.setFont(*menuFont);
	killFeedText.setCharacterSize(20);
	killFeedText.setColor(sf::Color::White);
	//killFeedText.setOrigin(0, killFeedText.getLocalBounds().height);

	pausedText.setFont(*menuFont);
	pausedText.setCharacterSize(80);
	pausedText.setColor(sf::Color::White);
	pausedText.setString("PAUSED");
	//pausedText.setPosition()

	titleScreenText.setFont(*menuFont);
	titleScreenText.setCharacterSize(80);
	titleScreenText.setColor(sf::Color::White);
	titleScreenText.setString("Press 'ENTER' to Start");
}

void hud::setUnitCostInfo(std::vector<std::string>* unitCostInfo)
{
	this->unitCostInfo = unitCostInfo;
}

float hud::getMenuHeight()
{
	return menuHeight;
}

void hud::setInfoBarSprite(sf::Texture* textureToSet, int windowSizeX, int windowSizeY, float squareSize, float infoBarHeight)
{
	infoBarSprite.setTexture(*textureToSet);
	infoBarSprite.setScale(windowSizeX * squareSize / textureToSet->getSize().x, infoBarHeight / textureToSet->getSize().y);
	infoBarSprite.setPosition(0, windowSizeY * squareSize - infoBarHeight);
}

void hud::setMinimapTexture(Game* game, sf::Texture* textToSet)
{
	minimap.setTexture(*textToSet);
	minimap.setScale((((float)menuHeight / (float)game->GAME_WORLD_SIZE_Y) * (float)game->GAME_WORLD_SIZE_X) / textToSet->getSize().x, menuHeight / textToSet->getSize().y);
	minimap.setPosition(0, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight);
}

void hud::resizeMinimap(Game* game)
{
	minimap.setScale((((float)menuHeight / (float)game->GAME_WORLD_SIZE_Y) * (float)game->GAME_WORLD_SIZE_X) / minimap.getTexture()->getSize().x, menuHeight / minimap.getTexture()->getSize().y);
	minimap.setPosition(0, game->videoModeY - game->UNIT_INFO_BOX_HEIGHT - menuHeight);
}

void hud::giveTItleScreenATexture(Game* game, sf::Texture* textToSet)
{
	titleScreen.setTexture(*textToSet);
	infoBarSprite.setScale(game->videoModeX / textToSet->getSize().x, game->videoModeY / textToSet->getSize().y);
}