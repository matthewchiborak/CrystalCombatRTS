#include "../include/renderer.h"
#include "../include/game.h"

Renderer::Renderer(Game *game, sf::RenderWindow* passedWindow, int windowSizeX, int windowSizeY, int windowPosX, int windowPosY, float squareSize, float minZoom, float maxZoom, int worldSizeX, int worldSizeY, sf::Font* menuFont)
{
	this->game = game;
	gameWindow = passedWindow;
	this->windowPosX = windowPosX;
	this->windowPosY = windowPosY;
	this->windowSizeX = windowSizeX;
	this->windowSizeY = windowSizeY;
	this->squareSize = squareSize;
	zoom = 1.0f;
	this->minZoom = minZoom;
	this->maxZoom = maxZoom;
	HUD.reset(sf::FloatRect(windowPosX * squareSize, windowPosY * squareSize, windowSizeX * squareSize, windowSizeY * squareSize));
	mainView.reset(sf::FloatRect(windowPosX * squareSize, windowPosY * squareSize, windowSizeX * squareSize, windowSizeY * squareSize));

	windowScrollOffset[0] = 0;
	windowScrollOffset[1] = 0;

	this->worldSizeX = worldSizeX;
	this->worldSizeY = worldSizeY;

	this->menuFont = menuFont;
	playerInterface.assignFont(menuFont);

	bValueX = -0.5 * windowSizeX;
	cValueX = 0.5 * windowSizeX;
	bValueY = -0.5 * windowSizeY;
	cValueY = 0.5 * windowSizeY;

	backgroundTiles = new world_tile*[worldSizeX];
	fogOfWar = new Fog*[worldSizeX];
	for (int i = 0; i < worldSizeX; i++)
	{
		backgroundTiles[i] = new world_tile[worldSizeY];
		fogOfWar[i] = new Fog[worldSizeY];
	}
}

Renderer::~Renderer()
{
	//Delete all the textures that were dynamically created in the vector of textures
	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		delete playerInGameObjects.at(i);
	}
	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		delete opponentInGameObjects.at(i);
	}

	for (int i = 0; i < worldSizeX; i++)
	{
		delete [] backgroundTiles[i];
		delete [] fogOfWar[i];
	}


	delete [] backgroundTiles;


}

GameObject * Renderer::addObjectToScene(const bool isPlayer, bool isStructure, const bool beingBuilt, const std::string type, const int xPos, const int yPos, const int sizeX, const int sizeY, sf::Texture* textureToSet, const float health, const float speed, const int visionRange, const int attackRange, const float attackPower, const float attackSpeed, const bool flying, const std::string createsType)
{
	//Check if not outside the world
	if (xPos + sizeX > worldSizeX || yPos + sizeY > worldSizeY)
	{
		return false;
	}

	//GameObject placeHolderObj;
	GameObject* newGameObject = new GameObject(isPlayer, isStructure, beingBuilt, type, xPos, yPos, sizeX, sizeY, squareSize, textureToSet, health, speed, selectionTexture, visionRange, attackRange, attackPower, attackSpeed, flying, createsType);
	newGameObject->setVisionMask(visionRange, squareSize);

	//If is larger than one tile, try to claim the other tiles arround it, if fails should release the other tiles around it
	for (int i = 0; i < sizeX; i++)
	{
		for (int j = 0; j < sizeY; j++)
		{
			if (!backgroundTiles[xPos + i][yPos + j].tryClaimTile(newGameObject))
			{
				//Failed to claim a tile

				//Release tile already claimed
				for (int ii = 0; ii <= i; ii++)
				{
					for (int jj = 0; jj <= j; jj++)
					{
						//Return after found the tile that was already claimed
						if (ii == i && jj == j)
						{
							return false;
						}
						else
						{
							backgroundTiles[xPos + i][yPos + j].freeTile(newGameObject);
						}
					}
				}

				delete newGameObject;
				return NULL;
			}
		}
	}

	/*GameObject* newGameObject = new GameObject(isPlayer, isStructure, beingBuilt, type, xPos, yPos, sizeX, sizeY, squareSize, textureToSet, health, speed, &selectionTexture, visionRange, attackRange, attackPower, attackSpeed);
	newGameObject->setVisionMask(visionRange, squareSize);*/

	//Claim the tiles
	/*for (int i = 0; i < sizeX; i++)
	{
		for (int j = 0; j < sizeY; j++)
		{
			backgroundTiles[xPos + i][yPos + j].setOccupiedObj(newGameObject);
		}
	}*/

	//Take the lock
	while (!gameObjectLock.try_lock()) 
	{
		//Keep trying to take the lock until it can
	}

	if (isPlayer)
	{
		playerInGameObjects.push_back(newGameObject);

		//If its a structure, also add it to the structure only vector for better effeciency
		if (isStructure)
		{
			structureRefsPlayer.push_back(newGameObject);
		}

		
	}
	else
	{
		opponentInGameObjects.push_back(newGameObject);

		//If its a structure, also add it to the structure only vector for better effeciency
		if (isStructure)
		{
			structureRefsOpponent.push_back(newGameObject);
		}

		for (int i = -1 * visionRange; i <= visionRange; i++)
		{
			for (int j = -1 * visionRange; j <= visionRange; j++)
			{
				if (xPos + i < worldSizeX && yPos + j < worldSizeY && xPos + i >= 0 && yPos + j >= 0)
				{
					fogOfWar[xPos + i][yPos + j].setEnabledOpponent(false);
					fogOfWar[xPos + i][yPos + j].setCurrentVisibleOpponent(true);
				}
			}
		}
	}
	
	//Release the lock
	gameObjectLock.unlock();

	// If the unit is a structure we update the pathmap
	if (newGameObject->getStructure())
	{
		PathMap *pathMap = PathMap::getPathMap();
		while (!pathMap->tryLock());
		for (int x = 0; x < newGameObject->getGridSizeX(); x++)
		{
			for (int y = 0; y < newGameObject->getGridSizeY(); y++)
			{
				// Set cost to negative - impassable (Like a wall)
				pathMap->setCost(
					newGameObject->getGridPosX() + x,
					newGameObject->getGridPosY() + y,
					-1);
			}
		}
		pathMap->unlock();
	}

	return newGameObject;
}

void Renderer::addObjectToScene(const bool isPlayer, GameObject* gameObjectToAdd)
{
	
}

void Renderer::drawScene(Game *game)
{
	//Get the lock on the game objects
	while (!gameObjectLock.try_lock());

	gameWindow->setActive(true);
	gameWindow->setView(mainView);
	drawBackgroundTiles();
	drawObjectWithView(game);
	drawFogOfWar();

	//Draw the HUD last
	drawHUD(game);

	gameWindow->setActive(false);

	//Release the lock
	gameObjectLock.unlock();
}

void Renderer::drawBackgroundTiles()
{
	gameWindow->draw(backgroundImage);
}

void Renderer::tellResourceTilesToMakeResources()
{
	for (int i = 0; i < resourceTiles.size(); i++)
	{
		resourceTiles.at(i)->createResources(pickupTexture, squareSize);
	}
}

void Renderer::tellResourceTilesToTryToMakeResources()
{
	for (int i = 0; i < resourceTiles.size(); i++) 
	{
		resourceTiles.at(i)->tryCreateResource(pickupTexture, squareSize);
	}
}

void Renderer::setFogOfWarTextureForMasking(sf::Texture* textureToSet)
{
	largeFogOfWar.setTexture(*textureToSet);
	largeFogOfWar.setScale(sf::Vector2f(worldSizeX * (squareSize) / textureToSet->getSize().x, worldSizeY * (squareSize) / textureToSet->getSize().y));
	largeForOfWarTexture = textureToSet;
}

void Renderer::setBackgroundTexture(sf::Texture* textureToSet, Game* game)
{
	backgroundImage.setTexture(*textureToSet);
	backgroundImage.setScale(sf::Vector2f(worldSizeX * (squareSize) / textureToSet->getSize().x, worldSizeY * (squareSize) / textureToSet->getSize().y));

	playerInterface.setMinimapTexture(game, textureToSet);
}

void Renderer::drawFogOfWar()
{
	resetVision();

	for (int i = 0; i < worldSizeX; i++)
	{
		for (int j = 0; j < worldSizeY; j++)
		{
			//Draw it
			if (fogOfWar[i][j].getEnabled())
			{
				
			}
			else if (!fogOfWar[i][j].getCurrentVisiblePlayer())
			{
				//Get the sprite of the last sprite that was seen there

				if (fogOfWar[i][j].getLastSeenIdPlayer() >= 0)
				{
					for (int lastIndex = 0; lastIndex < opponentInGameObjects.size(); lastIndex++)
					{
						if (fogOfWar[i][j].getLastSeenIdPlayer() == opponentInGameObjects.at(lastIndex)->getID())
						{
							fogOfWar[i][j].copyASpriteToTheLastSeen(opponentInGameObjects.at(lastIndex)->getSprite(), squareSize, i, j);

							//Only draw the last seen sprite if it is the root tile of the object. Prevents objects larger than one tile from being draw multiple times
							if (fogOfWar[i][j].getLastSeenCordOccupies())
							{
								gameWindow->draw(fogOfWar[i][j].getLastSpriteSeen());
							}
							break;
						}
					}
				}

			}
			else
			{
				//Currently visible by player
			}
		}
	}

	//Gunna try masking instead the large fog of war instead
	sf::RenderTexture visionHole;
	visionHole.create(worldSizeX * squareSize, worldSizeY * squareSize);
	visionHole.clear(sf::Color::Transparent);

	visionHole.draw(largeFogOfWar);

	//Also need to cut out from the non transparent fog of war
	//sf::RenderTexture visionHoleFog;
	//visionHoleFog.create(worldSizeX * squareSize, worldSizeY * squareSize);
	//visionHoleFog.clear(sf::Color::Transparent);

	//visionHoleFog.draw(largeFog);

	for (int pInd = 0; pInd < playerInGameObjects.size(); pInd++)
	{
		if (playerInGameObjects.at(pInd)->getActive())
		{
			sf::CircleShape* maskToDraw = playerInGameObjects.at(pInd)->getVisionMask();

			maskToDraw->setPosition(sf::Vector2f(playerInGameObjects.at(pInd)->getPreviousPosX() * (squareSize)+(playerInGameObjects.at(pInd)->getGridSizeX() * squareSize / 2.0) + playerInGameObjects.at(pInd)->getOffsetX(), playerInGameObjects.at(pInd)->getPreviousPosY() * (squareSize)+(playerInGameObjects.at(pInd)->getGridSizeY() * squareSize / 2.0) + playerInGameObjects.at(pInd)->getOffsetY()));

			visionHole.draw(*maskToDraw);
			//visionHoleFog.draw(*maskToDraw);

		}
	}

	visionHole.display();
	//visionHoleFog.display();

	sf::Image holeImage;
	holeImage = visionHole.getTexture().copyToImage();
	holeImage.createMaskFromColor(sf::Color::Magenta);

	//sf::Image holeImageFog;
	//holeImageFog = visionHoleFog.getTexture().copyToImage();
	//holeImageFog.createMaskFromColor(sf::Color::Magenta);

	sf::Texture tex;
	tex.loadFromImage(holeImage); 

	//sf::Texture fogTex;
	//fogTex.loadFromImage(holeImageFog);

	//Draw the transparentcy for the fog
	gameWindow->draw(sf::Sprite(tex));

	//largeFog = sf::Sprite(fogTex);

	//Draw the large fog
	//gameWindow->draw(largeFog);
}



void Renderer::drawObjectWithView(Game *game)
{
	float x = game->mousePos.x;
	float y = game->mousePos.y;

	int minVisibileX = windowPosX + bValueX * zoom + cValueX + ((0 + windowScrollOffset[0]) / (squareSize / zoom));
	int maxVisibileX = windowPosX + bValueX * zoom + cValueX + ((windowSizeX * squareSize + windowScrollOffset[0]) / (squareSize / zoom));
	int minVisibileY = windowPosY + bValueY * zoom + cValueY + ((0 + windowScrollOffset[1]) / (squareSize / zoom));
	int maxVisibileY = windowPosY + bValueY * zoom + cValueY + ((windowSizeY * squareSize + windowScrollOffset[1]) / (squareSize / zoom));

	//Draw all the structures first
	for (int i = 0; i < structureRefsOpponent.size(); i++)
	{
		//Only draw if it is on screen
		if (structureRefsOpponent.at(i)->getGridPosX() + structureRefsOpponent.at(i)->getGridSizeX() >= minVisibileX
			&& structureRefsOpponent.at(i)->getGridPosX() < maxVisibileX + 1
			&& structureRefsOpponent.at(i)->getGridPosY() + structureRefsOpponent.at(i)->getGridSizeY() >= minVisibileY
			&& structureRefsOpponent.at(i)->getGridPosY() < maxVisibileY + 1)
		{

			//See if any of the tile occupied by the object are visible to the player
			bool isVisible = false;
			if (structureRefsOpponent.at(i)->getActive())
			{
				for (int w = 0; w < structureRefsOpponent.at(i)->getGridSizeX(); w++)
				{
					for (int h = 0; h < structureRefsOpponent.at(i)->getGridSizeY(); h++)
					{
						if (fogOfWar[structureRefsOpponent.at(i)->getGridPosX() + w][structureRefsOpponent.at(i)->getGridPosY() + h].getCurrentVisiblePlayer())
						{
							isVisible = true;
							break;
						}
					}

					if (isVisible)
					{
						break;
					}
				}
			}

			{
				if (isVisible)
				{
					sf::Sprite& spriteToBeDrawn = structureRefsOpponent.at(i)->getSprite();

					//Set the position. Because origin isin the center of the object I need to add half its size to make it fit in the grid square
					spriteToBeDrawn.setPosition(sf::Vector2f(structureRefsOpponent.at(i)->getPreviousPosX() * (squareSize)+(structureRefsOpponent.at(i)->getGridSizeX() * squareSize / 2.0) + structureRefsOpponent.at(i)->getOffsetX(), structureRefsOpponent.at(i)->getPreviousPosY() * (squareSize)+(structureRefsOpponent.at(i)->getGridSizeY() * squareSize / 2.0) + structureRefsOpponent.at(i)->getOffsetY()));

					//Get the scale of the object
					spriteToBeDrawn.setScale(sf::Vector2f(((float)structureRefsOpponent.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().x, ((float)structureRefsOpponent.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().y));


					//Rotate the sprite
					spriteToBeDrawn.setRotation(structureRefsOpponent.at(i)->getRotation());

					//Draw it
					if (!structureRefsOpponent.at(i)->getBeingBuilt())
					{
						gameWindow->draw(spriteToBeDrawn);
					}


					//If the object is carrying resources, draw them
					if (structureRefsOpponent.at(i)->getPickupCarried())
					{
						//Move its position so that its in the correct spot
						structureRefsOpponent.at(i)->getPickupCarried()->getSprite()->setPosition(structureRefsOpponent.at(i)->getPreviousPosX() * squareSize + structureRefsOpponent.at(i)->getOffsetX(), structureRefsOpponent.at(i)->getPreviousPosY() * squareSize + structureRefsOpponent.at(i)->getOffsetY());

						gameWindow->draw(*structureRefsOpponent.at(i)->getPickupCarried()->getSprite());
					}
				}
			}
		}
	}



	//Draw all the player structures
	for (int i = 0; i < structureRefsPlayer.size(); i++)
	{
		if (structureRefsPlayer.at(i)->getActive())
		{
			//Only draw if it is on screen

			if (structureRefsPlayer.at(i)->getGridPosX() + structureRefsPlayer.at(i)->getGridSizeX() >= minVisibileX
				&& structureRefsPlayer.at(i)->getGridPosX() < maxVisibileX + 1
				&& structureRefsPlayer.at(i)->getGridPosY() + structureRefsPlayer.at(i)->getGridSizeY() >= minVisibileY
				&& structureRefsPlayer.at(i)->getGridPosY() < maxVisibileY + 1)
			{

				sf::Sprite& spriteToBeDrawn = structureRefsPlayer.at(i)->getSprite();

				//Set the position. Because origin isin the center of the object I need to add half its size to make it fit in the grid square
				spriteToBeDrawn.setPosition(sf::Vector2f(structureRefsPlayer.at(i)->getPreviousPosX() * (squareSize)+(structureRefsPlayer.at(i)->getGridSizeX() * squareSize / 2.0) + structureRefsPlayer.at(i)->getOffsetX(), structureRefsPlayer.at(i)->getPreviousPosY() * (squareSize)+(structureRefsPlayer.at(i)->getGridSizeY() * squareSize / 2.0) + structureRefsPlayer.at(i)->getOffsetY()));

				//Get the scale of the object
				spriteToBeDrawn.setScale(sf::Vector2f(((float)structureRefsPlayer.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().x, ((float)structureRefsPlayer.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().y));


				//Rotate the sprite
				spriteToBeDrawn.setRotation(structureRefsPlayer.at(i)->getRotation());

				//Draw it
				if (!structureRefsPlayer.at(i)->getBeingBuilt())
				{
					gameWindow->draw(spriteToBeDrawn);
				}


				//If the object is carrying resources, draw them
				if (structureRefsPlayer.at(i)->getPickupCarried())
				{
					//Move its position so that its in the correct spot
					structureRefsPlayer.at(i)->getPickupCarried()->getSprite()->setPosition(structureRefsPlayer.at(i)->getPreviousPosX() * squareSize + structureRefsPlayer.at(i)->getOffsetX(), structureRefsPlayer.at(i)->getPreviousPosY() * squareSize + structureRefsPlayer.at(i)->getOffsetY());

					gameWindow->draw(*structureRefsPlayer.at(i)->getPickupCarried()->getSprite());
				}

				//If mouse is over it, have it light up with the icon

				//If mouse is over it or the object is selected, have it light up with the icon

				int xCord = -1;
				int yCord = -1;
				xCord = windowPosX + bValueX * zoom + cValueX + ((x + windowScrollOffset[0]) / (squareSize / zoom));
				yCord = windowPosY + bValueY * zoom + cValueY + ((y + windowScrollOffset[1]) / (squareSize / zoom));


				int wantedId = -1;

			}
		}//If active
	}


	//Check if the space it is in is visibile to the player, then draw if it is
	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		//Only draw if it is on screen
		if (!opponentInGameObjects.at(i)->getStructure()
			&& opponentInGameObjects.at(i)->getGridPosX() + opponentInGameObjects.at(i)->getGridSizeX() >= minVisibileX
			&& opponentInGameObjects.at(i)->getGridPosX() < maxVisibileX + 1
			&& opponentInGameObjects.at(i)->getGridPosY() + opponentInGameObjects.at(i)->getGridSizeY() >= minVisibileY
			&& opponentInGameObjects.at(i)->getGridPosY() < maxVisibileY + 1)
		{

			//See if any of the tile occupied by the object are visible to the player
			bool isVisible = false;
			if (opponentInGameObjects.at(i)->getActive())
			{
				for (int w = 0; w < opponentInGameObjects.at(i)->getGridSizeX(); w++)
				{
					for (int h = 0; h < opponentInGameObjects.at(i)->getGridSizeY(); h++)
					{
						if (fogOfWar[opponentInGameObjects.at(i)->getGridPosX() + w][opponentInGameObjects.at(i)->getGridPosY() + h].getCurrentVisiblePlayer())
						{
							isVisible = true;
							break;
						}
					}

					if (isVisible)
					{
						break;
					}
				}
			}

			{
				if (isVisible)
				{
					sf::Sprite& spriteToBeDrawn = opponentInGameObjects.at(i)->getSprite();

					//Set the position. Because origin isin the center of the object I need to add half its size to make it fit in the grid square
					spriteToBeDrawn.setPosition(sf::Vector2f(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+(opponentInGameObjects.at(i)->getGridSizeX() * squareSize / 2.0) + opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+(opponentInGameObjects.at(i)->getGridSizeY() * squareSize / 2.0) + opponentInGameObjects.at(i)->getOffsetY()));

					//Get the scale of the object
					spriteToBeDrawn.setScale(sf::Vector2f(((float)opponentInGameObjects.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().x, ((float)opponentInGameObjects.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().y));


					//Rotate the sprite
					spriteToBeDrawn.setRotation(opponentInGameObjects.at(i)->getRotation());

					//Draw it
					if (!opponentInGameObjects.at(i)->getBeingBuilt())
					{
						gameWindow->draw(spriteToBeDrawn);
					}


					//If the object is carrying resources, draw them
					if (opponentInGameObjects.at(i)->getPickupCarried())
					{
						//Move its position so that its in the correct spot
						opponentInGameObjects.at(i)->getPickupCarried()->getSprite()->setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * squareSize + opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * squareSize + opponentInGameObjects.at(i)->getOffsetY());

						gameWindow->draw(*opponentInGameObjects.at(i)->getPickupCarried()->getSprite());
					}
				}
			}
		}
	}//End of for loop



	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		//Only draw the object if it is within view
		
		{
			
			{
				
				{
					
					if(playerInGameObjects.at(i)->getActive())
					{
						//Only draw if it is on screen
						
						if (!playerInGameObjects.at(i)->getStructure()
							&& playerInGameObjects.at(i)->getGridPosX() + playerInGameObjects.at(i)->getGridSizeX() >= minVisibileX
							&& playerInGameObjects.at(i)->getGridPosX() < maxVisibileX + 1
							&& playerInGameObjects.at(i)->getGridPosY() + playerInGameObjects.at(i)->getGridSizeY() >= minVisibileY
							&& playerInGameObjects.at(i)->getGridPosY() < maxVisibileY + 1)
						{

							sf::Sprite& spriteToBeDrawn = playerInGameObjects.at(i)->getSprite();

							//Set the position. Because origin isin the center of the object I need to add half its size to make it fit in the grid square
							spriteToBeDrawn.setPosition(sf::Vector2f(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeX() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeY() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetY()));

							//Get the scale of the object
							spriteToBeDrawn.setScale(sf::Vector2f(((float)playerInGameObjects.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().x, ((float)playerInGameObjects.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawn.getTexture()->getSize().y));


							//Rotate the sprite
							spriteToBeDrawn.setRotation(playerInGameObjects.at(i)->getRotation());

							//Draw it
							if (!playerInGameObjects.at(i)->getBeingBuilt())
							{
								gameWindow->draw(spriteToBeDrawn);
							}


							//If the object is carrying resources, draw them
							if (playerInGameObjects.at(i)->getPickupCarried())
							{
								//Move its position so that its in the correct spot
								playerInGameObjects.at(i)->getPickupCarried()->getSprite()->setPosition(playerInGameObjects.at(i)->getPreviousPosX() * squareSize + playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * squareSize + playerInGameObjects.at(i)->getOffsetY());

								gameWindow->draw(*playerInGameObjects.at(i)->getPickupCarried()->getSprite());
							}

							//If mouse is over it, have it light up with the icon

							//If mouse is over it or the object is selected, have it light up with the icon

							int xCord = -1;
							int yCord = -1;
							xCord = windowPosX + bValueX * zoom + cValueX + ((x + windowScrollOffset[0]) / (squareSize / zoom));
							yCord = windowPosY + bValueY * zoom + cValueY + ((y + windowScrollOffset[1]) / (squareSize / zoom));
							

							int wantedId = -1;

							//If it was currently selected, keep the selection icon
							//if (game->currentMultiSelectedObjects.size() > 0 &&
							//	game->currentMultiSelectedObjects[0]->getID() == playerInGameObjects.at(i)->getID())
							//{
							//	sf::Sprite& spriteToBeDrawnSelection = playerInGameObjects.at(i)->getSelectionTexture();

							//	spriteToBeDrawnSelection.setPosition(sf::Vector2f(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeX() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeY() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetY()));

							//	//Get the scale of the object
							//	spriteToBeDrawnSelection.setScale(sf::Vector2f(((float)playerInGameObjects.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().x, ((float)playerInGameObjects.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().y));

							//	//Draw it
							//	gameWindow->draw(spriteToBeDrawnSelection);
							//}

							//if (xCord >= 0 && yCord >= 0 && xCord < worldSizeX && yCord < worldSizeY)
							//{
							//	
							//	if (backgroundTiles[xCord][yCord].getOccupiedObj())
							//	{
							//		//if (backgroundTiles[xCord][yCord].getOccupiedObj()->getID() == playerInGameObjects.at(i)->getID())
							//		if (backgroundTiles[xCord][yCord].checkIfThisObjInHere(playerInGameObjects.at(i)))
							//		{
							//			sf::Sprite& spriteToBeDrawnSelection = playerInGameObjects.at(i)->getSelectionTexture();

							//			spriteToBeDrawnSelection.setPosition(sf::Vector2f(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeX() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+(playerInGameObjects.at(i)->getGridSizeY() * squareSize / 2.0) + playerInGameObjects.at(i)->getOffsetY()));

							//			//Get the scale of the object
							//			spriteToBeDrawnSelection.setScale(sf::Vector2f(((float)playerInGameObjects.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().x, ((float)playerInGameObjects.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().y));

							//			//Draw it
							//			gameWindow->draw(spriteToBeDrawnSelection);
							//		}
							//	}
							//}
						}
					}//If active
				}
			}
		}
	}

	//Draw the healthbars
	//Check if the space it is in is visibile to the player, then draw if it is
	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		//Only draw if it is on screen
		if (opponentInGameObjects.at(i)->getGridPosX() + opponentInGameObjects.at(i)->getGridSizeX() >= minVisibileX
			&& opponentInGameObjects.at(i)->getGridPosX() < maxVisibileX + 1
			&& opponentInGameObjects.at(i)->getGridPosY() + opponentInGameObjects.at(i)->getGridSizeY() >= minVisibileY
			&& opponentInGameObjects.at(i)->getGridPosY() < maxVisibileY + 1)
		{

			//See if any of the tile occupied by the object are visible to the player
			bool isVisible = false;
			if (opponentInGameObjects.at(i)->getActive())
			{
				for (int w = 0; w < opponentInGameObjects.at(i)->getGridSizeX(); w++)
				{
					for (int h = 0; h < opponentInGameObjects.at(i)->getGridSizeY(); h++)
					{
						if (fogOfWar[opponentInGameObjects.at(i)->getGridPosX() + w][opponentInGameObjects.at(i)->getGridPosY() + h].getCurrentVisiblePlayer())
						{
							isVisible = true;
							break;
						}
					}

					if (isVisible)
					{
						break;
					}
				}
			}

			{
				if (isVisible)
				{
					//Now draw the game object's health
					sf::RectangleShape redHealth;
					redHealth.setSize(sf::Vector2f(opponentInGameObjects.at(i)->getGridSizeX() * squareSize, squareSize * HEALTH_BAR_SCALE));
					
					redHealth.setFillColor(sf::Color(0, 0, 0, 205));

					if (!opponentInGameObjects.at(i)->getBeingBuilt())
						redHealth.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() - HEALTH_BAR_OFFSET);
					else
						redHealth.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() + squareSize / 2);


					gameWindow->draw(redHealth);

					sf::RectangleShape greenHealth;
					greenHealth.setSize(sf::Vector2f(opponentInGameObjects.at(i)->getGridSizeX() * opponentInGameObjects.at(i)->getHealthPercentage() * squareSize, squareSize * HEALTH_BAR_SCALE));
				
					if (opponentInGameObjects.at(i)->getStructure())
					{
						
						greenHealth.setFillColor(sf::Color(255, 0, 0, 205));
					}
					else
					{
						greenHealth.setFillColor(sf::Color(255, 0, 0, 205));
					}

					if (!opponentInGameObjects.at(i)->getBeingBuilt())
						greenHealth.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() - HEALTH_BAR_OFFSET);
					else
						greenHealth.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() + squareSize / 2);


					gameWindow->draw(greenHealth);

					//Now if it is collecting resources, draw its current progress on the next resource
					if (opponentInGameObjects.at(i)->getIsCollecting())
					{
						sf::RectangleShape collectBack;
						collectBack.setSize(sf::Vector2f(opponentInGameObjects.at(i)->getGridSizeX() * squareSize, squareSize * HEALTH_BAR_SCALE));

						collectBack.setFillColor(sf::Color(0, 0, 0, 205));
						collectBack.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() + squareSize / 2);

						gameWindow->draw(collectBack);

						sf::RectangleShape collectFront;
						collectFront.setSize(sf::Vector2f(opponentInGameObjects.at(i)->getGridSizeX() * opponentInGameObjects.at(i)->getPickupPercentage() * squareSize, squareSize * HEALTH_BAR_SCALE));
						collectFront.setFillColor(sf::Color(138, 43, 226, 205));

						collectFront.setPosition(opponentInGameObjects.at(i)->getPreviousPosX() * (squareSize)+opponentInGameObjects.at(i)->getOffsetX(), opponentInGameObjects.at(i)->getPreviousPosY() * (squareSize)+opponentInGameObjects.at(i)->getOffsetY() + squareSize / 2);
						
						if(!game->paused)
						gameWindow->draw(collectFront);
					}

					
				}
			}
		}
	}//End of for loop

	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		//Only draw the object if it is within view
		
		{
			
			{
				
				{
					
					if (playerInGameObjects.at(i)->getActive())
					{
						//Only draw if it is on screen
						
						if (playerInGameObjects.at(i)->getGridPosX() + playerInGameObjects.at(i)->getGridSizeX() >= minVisibileX
							&& playerInGameObjects.at(i)->getGridPosX() < maxVisibileX + 1
							&& playerInGameObjects.at(i)->getGridPosY() + playerInGameObjects.at(i)->getGridSizeY() >= minVisibileY
							&& playerInGameObjects.at(i)->getGridPosY() < maxVisibileY + 1)
						{

							


							//Now draw the game object's health
							sf::RectangleShape redHealth;
							redHealth.setSize(sf::Vector2f(playerInGameObjects.at(i)->getGridSizeX() * squareSize, squareSize * HEALTH_BAR_SCALE));
							
							redHealth.setFillColor(sf::Color(0, 0, 0, 205));

							if (playerInGameObjects.at(i)->getBeingBuilt())
								redHealth.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() + squareSize / 2);
							else
								redHealth.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() - HEALTH_BAR_OFFSET);

							gameWindow->draw(redHealth);

							sf::RectangleShape greenHealth;
							greenHealth.setSize(sf::Vector2f(playerInGameObjects.at(i)->getGridSizeX() * playerInGameObjects.at(i)->getHealthPercentage() * squareSize, squareSize * HEALTH_BAR_SCALE));
							
							if (playerInGameObjects.at(i)->getStructure())
							{
								greenHealth.setFillColor(sf::Color(0, 191, 255, 205));
							}
							else
							{
								greenHealth.setFillColor(sf::Color(0, 255, 0, 205));
							}

							if (!playerInGameObjects.at(i)->getBeingBuilt())
								greenHealth.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() - HEALTH_BAR_OFFSET);
							else
								greenHealth.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() + squareSize / 2);



							gameWindow->draw(greenHealth);

							//Now if it is collecting resources, draw its current progress on the next resource
							if (playerInGameObjects.at(i)->getIsCollecting())
							{
								sf::RectangleShape collectBack;
								collectBack.setSize(sf::Vector2f(playerInGameObjects.at(i)->getGridSizeX() * squareSize, squareSize * HEALTH_BAR_SCALE));

								collectBack.setFillColor(sf::Color(0, 0, 0, 205));
								collectBack.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() + squareSize / 2);

								gameWindow->draw(collectBack);

								sf::RectangleShape collectFront;
								collectFront.setSize(sf::Vector2f(playerInGameObjects.at(i)->getGridSizeX() * playerInGameObjects.at(i)->getPickupPercentage() * squareSize, squareSize * HEALTH_BAR_SCALE));
								collectFront.setFillColor(sf::Color(138, 43, 226, 205));

								collectFront.setPosition(playerInGameObjects.at(i)->getPreviousPosX() * (squareSize)+playerInGameObjects.at(i)->getOffsetX(), playerInGameObjects.at(i)->getPreviousPosY() * (squareSize)+playerInGameObjects.at(i)->getOffsetY() + squareSize / 2);
								
								if(!game->paused)
								gameWindow->draw(collectFront);
							}

						}
					}//If active
				}
			}
		}
	}

	if(game->singledOutMultiSelectedObjectInd >= 0)
	{ 
		sf::Sprite& spriteToBeDrawnSelection = game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getSelectionTexture();

		spriteToBeDrawnSelection.setPosition(sf::Vector2f(game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getPreviousPosX() * (squareSize)+(game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getGridSizeX() * squareSize / 2.0) + game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getOffsetX(), game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getPreviousPosY() * (squareSize)+(game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getGridSizeY() * squareSize / 2.0) + game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getOffsetY()));

		//Get the scale of the object
		spriteToBeDrawnSelection.setScale(sf::Vector2f(((float)game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getGridSizeX() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().x, ((float)game->currentMultiSelectedObjects.at(game->singledOutMultiSelectedObjectInd)->getGridSizeY() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().y));

		//Draw it
		gameWindow->draw(spriteToBeDrawnSelection);
	}
	else
	{
		//Draw all the selection icons for the multiselected objects
		for (int i = 0; i < game->currentMultiSelectedObjects.size(); i++)
		{
			sf::Sprite& spriteToBeDrawnSelection = game->currentMultiSelectedObjects.at(i)->getSelectionTexture();

			spriteToBeDrawnSelection.setPosition(sf::Vector2f(game->currentMultiSelectedObjects.at(i)->getPreviousPosX() * (squareSize)+(game->currentMultiSelectedObjects.at(i)->getGridSizeX() * squareSize / 2.0) + game->currentMultiSelectedObjects.at(i)->getOffsetX(), game->currentMultiSelectedObjects.at(i)->getPreviousPosY() * (squareSize)+(game->currentMultiSelectedObjects.at(i)->getGridSizeY() * squareSize / 2.0) + game->currentMultiSelectedObjects.at(i)->getOffsetY()));

			//Get the scale of the object
			spriteToBeDrawnSelection.setScale(sf::Vector2f(((float)game->currentMultiSelectedObjects.at(i)->getGridSizeX() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().x, ((float)game->currentMultiSelectedObjects.at(i)->getGridSizeY() * (squareSize)) / spriteToBeDrawnSelection.getTexture()->getSize().y));

			//Draw it
			gameWindow->draw(spriteToBeDrawnSelection);
		}
	}
}

GameObject* Renderer::getGameObject(bool isPlayer, int index)
{
	if (isPlayer)
	{
		return playerInGameObjects.at(index);
	}

	return opponentInGameObjects.at(index);
}

GameObject* Renderer::getGameObjectWithId(bool isPlayer, int objectId)
{
	if (isPlayer)
	{
		for (int i = 0; i < playerInGameObjects.size(); i++)
		{
			if (playerInGameObjects.at(i)->getID() == objectId)
			{
				return playerInGameObjects.at(i);
			}
		}
	}
	else
	{
		for (int i = 0; i < opponentInGameObjects.size(); i++)
		{
			if (opponentInGameObjects.at(i)->getID() == objectId)
			{
				return opponentInGameObjects.at(i);
			}
		}
	}

	return NULL;
}


world_tile * Renderer::getTile(const int xCord, const int yCord)
{
	if (xCord < 0 || yCord < 0 || xCord >= worldSizeX || yCord >= worldSizeY)
		return NULL;
	else
		return &backgroundTiles[xCord][yCord];
}

world_tile* Renderer::getNearestTileWithMouseCords(bool* foundTheTile, float x, float y)
{
	int xCord = -1;
	int yCord = -1;
	bool objectFound = false;

	//Find the tile
	//Need to add because zoomed at centre not edge of screen
	//The window postion plus the offset generated from zooming in the screen and then the offset of where the player clicked the screen
	xCord = windowPosX + bValueX * zoom + cValueX + ((x + windowScrollOffset[0]) / (squareSize / zoom));
	yCord = windowPosY + bValueY * zoom + cValueY + ((y + windowScrollOffset[1]) / (squareSize / zoom));

	if (xCord < 0)
	{
		xCord = 0;
	}
	if (yCord < 0)
	{
		yCord = 0;
	}
	if (xCord >= worldSizeX)
	{
		xCord = worldSizeX - 1;
	}
	if (yCord >= worldSizeY)
	{
		yCord = worldSizeY - 1;
	}

	//Get the tile that matches the cordinates
	*foundTheTile = true;
	return &backgroundTiles[xCord][yCord];
}

world_tile* Renderer::getTileWithMouseCords(bool* foundTheTile, float x, float y)
{
	int xCord = -1;
	int yCord = -1;
	bool objectFound = false;
	
	//Find the tile
	//Need to add because zoomed at centre not edge of screen
	//The window postion plus the offset generated from zooming in the screen and then the offset of where the player clicked the screen
	xCord = windowPosX + bValueX * zoom + cValueX + ((x + windowScrollOffset[0]) / (squareSize / zoom));
	yCord = windowPosY + bValueY * zoom + cValueY + ((y + windowScrollOffset[1]) / (squareSize / zoom));

	if (xCord < 0 || yCord < 0 || xCord >= worldSizeX || yCord >= worldSizeY)
	{
		//Not a square in the world
		*foundTheTile = false;
		return NULL;
	}
	
	//Get the tile that matches the cordinates
	*foundTheTile = true;
	return &backgroundTiles[xCord][yCord];
}

const int Renderer::getWorldSizeX()
{
	return worldSizeX;
}

const int Renderer::getWorldSizeY()
{
	return worldSizeY;
}

GameObject* Renderer::getGameObjectAtThisLocation(bool isPlayerObject, bool* foundTheObj, float x, float y)
{
	//Find what tile the player has clicked
	int xCord = -1;
	int yCord = -1;
	bool objectFound = false;

	//Find the tile
	//Need to add because zoomed at centre not edge of screen
	//The window postion plus the offset generated from zooming in the screen and then the offset of where the player clicked the screen
	xCord = windowPosX + bValueX * zoom + cValueX + ((x + windowScrollOffset[0]) / (squareSize / zoom));
	yCord = windowPosY + bValueY * zoom + cValueY + ((y + windowScrollOffset[1]) / (squareSize / zoom));

	if (xCord < 0 || yCord < 0 || xCord >= worldSizeX || yCord >= worldSizeY)
	{
		//Not a square in the world
		*foundTheObj = false;
		return nullptr;
	}


	GameObject* desiredObject = backgroundTiles[xCord][yCord].getOccupiedObj();

	if (desiredObject)
	{
		if (isPlayerObject && desiredObject->getIsPlayer())
		{
			*foundTheObj = true;
			return desiredObject;
		}
		else if (!isPlayerObject && !desiredObject->getIsPlayer())
		{
			*foundTheObj = true;
			return desiredObject;
		}
	}
	 
	return NULL;
}

bool Renderer::removeObjectFromScene(GameObject* gameObjectToRemove)
{
	//Search for the object based on its name
	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		if (playerInGameObjects.at(i)->getID() == gameObjectToRemove->getID())
		{
			

			//Free up tiles
			for (int i = 0; i < playerInGameObjects.at(i)->getGridSizeX(); i++)
			{
				for (int j = 0; j < playerInGameObjects.at(i)->getGridSizeY(); j++)
				{
					backgroundTiles[playerInGameObjects.at(i)->getGridPosX() + i][playerInGameObjects.at(i)->getGridPosY() + j].freeTile(playerInGameObjects.at(i));
				}
			}


			playerInGameObjects.at(i)->disable();
			return true;
		}
	}

	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		if (opponentInGameObjects.at(i)->getID() == gameObjectToRemove->getID())
		{
			//Free up tiles
			for (int i = 0; i < opponentInGameObjects.at(i)->getGridSizeX(); i++)
			{
				for (int j = 0; j < opponentInGameObjects.at(i)->getGridSizeY(); j++)
				{
					backgroundTiles[opponentInGameObjects.at(i)->getGridPosX() + i][opponentInGameObjects.at(i)->getGridPosY() + j].freeTile(opponentInGameObjects.at(i));
				}
			}

			opponentInGameObjects.at(i)->disable();
			return true;
		}
	}

	//Couldn't find the texture.
	return false;
}

bool Renderer::removeObjectFromScene(int id)
{
	//Search for the object based on its name

	// Player objects
	for (int i = playerInGameObjects.size() - 1; i >= 0; i--)
	{
		if (playerInGameObjects.at(i)->getID() == id)
		{
			backgroundTiles[playerInGameObjects.at(i)->getGridPosX()][playerInGameObjects.at(i)->getGridPosY()].freeTile(playerInGameObjects.at(i));

			for (int j = 0; j < playerInGameObjects.at(i)->getGridSizeX(); j++)
			{
				for (int k = 0; k < playerInGameObjects.at(i)->getGridSizeY(); k++)
				{
					backgroundTiles[playerInGameObjects.at(i)->getGridPosX() + j][playerInGameObjects.at(i)->getGridPosY() + k].freeTile(playerInGameObjects.at(i));
				}
			}

			// If structure, update pathmap
			if (playerInGameObjects[i]->getStructure())
			{
				PathMap *pathMap = PathMap::getPathMap();
				while (!pathMap->tryLock());
				for (int x = playerInGameObjects[i]->getGridPosX();
					x < playerInGameObjects[i]->getGridPosX() + playerInGameObjects[i]->getGridSizeX(); 
					x++)
				{
					for (int y = playerInGameObjects[i]->getGridPosY();
						y < playerInGameObjects[i]->getGridPosY() + playerInGameObjects[i]->getGridSizeY();
						y++)
					{
						// Set cost to negative - impassable (Like a wall)
						pathMap->setCost(x,	y, getTile(x, y)->getCost());
					}
				}
					pathMap->unlock();
			}

			playerInGameObjects[i]->disable();
			return true;
		}
	}

	// Opponent objects
	for (int i = opponentInGameObjects.size() - 1; i >= 0; i--)
	{
		if (opponentInGameObjects.at(i)->getID() == id)
		{
			for (int j = 0; j < opponentInGameObjects.at(i)->getGridSizeX(); j++)
			{
				for (int k = 0; k < opponentInGameObjects.at(i)->getGridSizeY(); k++)
				{
					backgroundTiles[opponentInGameObjects.at(i)->getGridPosX() + j][opponentInGameObjects.at(i)->getGridPosY() + k].freeTile(opponentInGameObjects.at(i));
				}
			}

			// If structure, update pathmap
			if (opponentInGameObjects[i]->getStructure())
			{
				PathMap *pathMap = PathMap::getPathMap();
				while (!pathMap->tryLock());
				for (int x = opponentInGameObjects[i]->getGridPosX();
				x < opponentInGameObjects[i]->getGridPosX() + opponentInGameObjects[i]->getGridSizeX();
					x++)
				{
					for (int y = opponentInGameObjects[i]->getGridPosY();
					y < opponentInGameObjects[i]->getGridPosY() + opponentInGameObjects[i]->getGridSizeY();
						y++)
					{
						// Set cost to negative - impassable (Like a wall)
						pathMap->setCost(x, y, getTile(x, y)->getCost());
					}
				}
				pathMap->unlock();
			}

			opponentInGameObjects.at(i)->disable();
			return true;
		}
	}

	//Couldn't find the texture.
	return false;
}

void Renderer::zoomView(float zoomIncrement)
{
	zoom += zoomIncrement;

	if (zoom > maxZoom)
	{
		zoom = maxZoom;
	}
	if (zoom < minZoom)
	{
		zoom = minZoom;
	}

	mainView.setSize(windowSizeX * squareSize * zoom, windowSizeY * squareSize * zoom);
}

void Renderer::setZoomView(float zoomValue)
{
	zoom = zoomValue;

	if (zoom > maxZoom)
	{
		zoom = maxZoom;
	}
	if (zoom < minZoom)
	{
		zoom = minZoom;
	}

	mainView.zoom(zoom);
}

void Renderer::giveBigFogATexture(sf::Texture* textToSet, float squareSize)
{
	fogTex = *textToSet;
	largeFog.setTexture(*textToSet);

	fogScaleX = (float)worldSizeX * (squareSize) / textToSet->getSize().x;
	fogScaleY = (float)worldSizeY * (squareSize) / (float)textToSet->getSize().y;

	largeFog.setScale(sf::Vector2f((float)worldSizeX * (squareSize) / textToSet->getSize().x, (float)worldSizeY * (squareSize) / (float)textToSet->getSize().y));
}
void Renderer::giveFogTilesATexture(int xCord, int yCord, sf::Texture* textToSet, float squareSize)
{
	fogOfWar[xCord][yCord].setTexture(textToSet, squareSize, xCord, yCord);
}
void Renderer::giveFogTilesATransparency(int xCord, int yCord, sf::Texture* textToSet, float squareSize)
{
	fogOfWar[xCord][yCord].setTransparency(textToSet, squareSize, xCord, yCord);
}
void Renderer::giveFogTilesABlankLastSeen(int xCord, int yCord, sf::Texture* textToSet, float squareSize)
{
	fogOfWar[xCord][yCord].setLastSeen(textToSet, squareSize, xCord, yCord);
}

void Renderer::setBackgroundTilePosition(const int xCord, const int yCord)
{
	backgroundTiles[xCord][yCord].setPosition(xCord, yCord);
}

void Renderer::moveCamera(float xMove, float yMove)
{
	windowScrollOffset[0] += xMove;
	windowScrollOffset[1] += yMove;

	if (windowScrollOffset[0] > squareSize / 2)
	{
		windowScrollOffset[0] = -(squareSize / 2);
		windowPosX++;
	}
	if (windowScrollOffset[0] < -(squareSize / 2))
	{
		windowScrollOffset[0] = (squareSize / 2);
		windowPosX--;
	}
	if (windowScrollOffset[1] > squareSize / 2)
	{
		windowScrollOffset[1] = -(squareSize / 2);
		windowPosY++;
	}
	if (windowScrollOffset[1] < -(squareSize / 2))
	{
		windowScrollOffset[1] = (squareSize / 2);
		windowPosY--;
	}

	mainView.move(xMove, yMove);
}

void Renderer::checkGameOver()
{
	if (!playerBase->getActive() || playerBase->getHealthPercentage() <= 0)
	{
		game->endTheGame(false);
		return;
	}
	if (!enemyBase->getActive() || enemyBase->getHealthPercentage() <= 0)
	{
		game->endTheGame(true);
		return;
	}
}

void Renderer::tellObjectsToDoActions(Game* game)
{
	//Check game over state
	if (!playerBase->getActive() || playerBase->getHealthPercentage() <= 0)
	{
		game->endTheGame(false);
		return;
	}
	if (!enemyBase->getActive() || enemyBase->getHealthPercentage() <= 0)
	{
		game->endTheGame(true);
		return;
	}

	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		GameObject *current = playerInGameObjects[i];

		//If a tile is visible to an object, tell the fog that it is
		if (current->getActive())
		{
			current->doAction(true, DEFAULT_VISION_RANGE, worldSizeX, worldSizeY, squareSize, backgroundTiles, fogOfWar);

			//Check if it is in attack range of its target and if so attack it
			GameObject *attackTarget = current->getAttackTarget();
			if (attackTarget)
			{
				if (attackTarget->getActive())
				{

					if (attackTargetInRange(current, attackTarget))
					{
						//In range. Attack
						//Increment the timer. If its time to attack, attack.
						if (attackTarget && playerInGameObjects.at(i)->tryAttack())
						{
							//GameObject *attackTarget = playerInGameObjects.at(i)->getAttackTarget();

							game->updateKillFeed("Enemy " + (attackTarget->getType()) + " " + std::to_string(attackTarget->getID()) + " took " + floatToString(playerInGameObjects.at(i)->getAttackPower(), 1) + " damage");

							if (attackTarget && attackTarget->doDamage(playerInGameObjects.at(i)->getAttackPower()))
							{
								checkGameOver();
								game->updateKillFeed("Enemy " + (attackTarget->getType()) + " " + std::to_string(attackTarget->getID()) + " died");

								//Release the tile
								for (int j = attackTarget->getGridPosX(); j < attackTarget->getGridPosX() + attackTarget->getGridSizeX(); j++)
								{
									for (int k = attackTarget->getGridPosY(); k < attackTarget->getGridPosY() + attackTarget->getGridSizeY(); k++)
									{
										backgroundTiles[j][k].freeTile(attackTarget);
									}
								}

								//If got to here, target is dead
								//Steal its resources if it has some
								if (attackTarget->getPickupCarried())
								{
									if (playerInGameObjects.at(i)->getPickupCarried())
									{
										playerInGameObjects.at(i)->getPickupCarried()->addToValue(attackTarget->getPickupCarried()->getValue());
										delete attackTarget->getPickupCarried();
									}
									else
									{
										playerInGameObjects.at(i)->setPickupCarried(attackTarget->getPickupCarried());
									}
								}

								// No target now
								playerInGameObjects.at(i)->setAttackTarget(NULL);

								// Kill the unit
								// If structure, update pathmap
								if (attackTarget->getStructure())
								{
									PathMap *pathMap = PathMap::getPathMap();
									while (!pathMap->tryLock());
									for (int x = attackTarget->getGridPosX();
										x < attackTarget->getGridPosX() + attackTarget->getGridSizeX();
										x++)
									{
										for (int y = attackTarget->getGridPosY();
											y < attackTarget->getGridPosY() + attackTarget->getGridSizeY();
											y++)
										{
											// Set cost to original cost
											pathMap->setCost(x, y, getTile(x, y)->getCost());
										}
									}
									pathMap->unlock();
								}

								// Clear reference
								for (int i = 0; i < opponentInGameObjects.size(); i++)
								{
									if (opponentInGameObjects[i] == attackTarget)
									{
										opponentInGameObjects.erase(opponentInGameObjects.begin() + i);
										break;
									}
								}

								if (attackTarget->getStructure())
								{
									// Remove from structures
									for (int i = 0; i < structureRefsOpponent.size(); i++)
									{
										if (structureRefsOpponent[i] == attackTarget)
										{
											structureRefsOpponent.erase(structureRefsOpponent.begin() + i);
											break;
										}
									}

									game->perceivedOpponent.structures--;
									game->actualOpponent.structures--;
								}
								else
								{
									game->perceivedOpponent.structures--;
									game->actualOpponent.units--;
								}

								game->killGameObject(attackTarget);
							}
						}
					}
					else if(!playerInGameObjects.at(i)->busy())
					{
						Game::moveToLocation(
							playerInGameObjects.at(i),
							game->pathList,
							getTile(
								playerInGameObjects.at(i)->getGridPosX(),
								playerInGameObjects.at(i)->getGridPosY()),
							getTile(
								playerInGameObjects.at(i)->getAttackTarget()->getGridPosX(),
								playerInGameObjects.at(i)->getAttackTarget()->getGridPosY()));
						
					}
				}
				else
				{
					playerInGameObjects.at(i)->setAttackTarget(NULL);
				}
			}
		}

	}

	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		GameObject *current = opponentInGameObjects[i];
		//If a tile is visible to an object, tell the fog that it is
		if (current->getActive())
		{
			current->doAction(false, DEFAULT_VISION_RANGE, worldSizeX, worldSizeY, squareSize, backgroundTiles, fogOfWar);

			GameObject *attackTarget = current->getAttackTarget();
			//Check if it is in attack range of its target and if so attack it
			if (attackTarget)
			{
				if (attackTarget->getActive())
				{
					//TODO make code update pathmap if structure and delete attackTarget if it dies

					if (attackTargetInRange(current, attackTarget))
					{
						//In range. Attack
						if (attackTarget && current->tryAttack())
						{
							game->updateKillFeed("Friendly " + (attackTarget->getType()) + " " + std::to_string(attackTarget->getID()) + " took " + floatToString(opponentInGameObjects.at(i)->getAttackPower()) + " damage");

							if (attackTarget->doDamage(current->getAttackPower()))
							{
								checkGameOver();

								game->updateKillFeed("Friendly " + (attackTarget->getType()) + " " + std::to_string(attackTarget->getID()) + " died");

								//Release the tile
								backgroundTiles[attackTarget->getGridPosX()][attackTarget->getGridPosY()].freeTile(attackTarget);

								for (int j = 0; j < attackTarget->getGridSizeX(); j++)
								{
									for (int k = 0; k < attackTarget->getGridSizeY(); k++)
									{
										backgroundTiles[attackTarget->getGridPosX() + j][attackTarget->getGridPosY() + k].freeTile(attackTarget);
									}
								}

								//If got to here, target is dead
								//Steal its resources if it has some
								if (attackTarget->getPickupCarried())
								{
									if (current->getPickupCarried())
									{
										current->getPickupCarried()->addToValue(attackTarget->getPickupCarried()->getValue());
										delete attackTarget->getPickupCarried();
									}
									else
									{
										current->setPickupCarried(attackTarget->getPickupCarried());
									}
								}
								current->setAttackTarget(NULL);

								// Kill the unit
								// If structure, update pathmap
								if (attackTarget->getStructure())
								{
									PathMap *pathMap = PathMap::getPathMap();
									while (!pathMap->tryLock());
									for (int x = attackTarget->getGridPosX();
									x < attackTarget->getGridPosX() + attackTarget->getGridSizeX();
										x++)
									{
										for (int y = attackTarget->getGridPosY();
										y < attackTarget->getGridPosY() + attackTarget->getGridSizeY();
											y++)
										{
											// Set cost to original cost
											pathMap->setCost(x, y, getTile(x, y)->getCost());
										}
									}
									pathMap->unlock();
								}

								// Clear reference
								for (int i = 0; i < playerInGameObjects.size(); i++)
								{
									if (playerInGameObjects[i] == attackTarget)
									{
										playerInGameObjects.erase(playerInGameObjects.begin() + i);
										break;
									}
								}

								if (attackTarget->getStructure())
								{
									for (int i = 0; i < structureRefsPlayer.size(); i++)
									{
										if (structureRefsPlayer[i] == attackTarget)
										{
											structureRefsPlayer.erase(structureRefsPlayer.begin() + i);
										}
									}

									game->perceivedPlayer.structures--;
									game->actualPlayer.structures--;
								}
								else
								{
									game->perceivedPlayer.units--;
									game->actualPlayer.units--;
								}

								game->killGameObject(attackTarget);
							}
						}
					}
				}
				else
				{
					opponentInGameObjects.at(i)->setAttackTarget(NULL);
				}
			}
		}
	}
}

void Renderer::setSelectionTexture(Game* game, sf::Texture* textureToSet, int* currentlySelectedMenu)
{
	selectionTexture = textureToSet;

	playerInterface.setSelectionTexture(game, textureToSet, windowSizeX, squareSize, MENU_SIZE_X, MENU_SIZE_Y, currentlySelectedMenu);
}

void Renderer::setVisionMaskTexture(sf::Texture* textureToSet)
{
	visionMaskTexture = *textureToSet;
}

//void Renderer::drawHUD(float x, float y, int playerResourceCount)
void Renderer::drawHUD(Game* game)
{
	gameWindow->setView(HUD);

	playerInterface.drawHUD(game, gameWindow, windowSizeX, windowSizeY, MENU_SIZE_X, MENU_SIZE_Y, backgroundTiles, fogOfWar, &playerInGameObjects, &opponentInGameObjects, windowPosX, windowPosY, zoom, bValueX, cValueX, bValueY, cValueY);

	//Don't forget to set back the view
	gameWindow->setView(mainView);
}

float Renderer::giveHUDSpriteATexture(Game* game, sf::Texture* textToSet, float squareSize, int index, int totalIcons, float infoHeight)
{
	return playerInterface.giveHUDSpriteATexture(game, textToSet, squareSize, windowSizeX, windowSizeY, MENU_SIZE_X, MENU_SIZE_Y, index, totalIcons, infoHeight);
}

void Renderer::giveHUDtheUnitCostInfo(std::vector<std::string>* unitCostInfo)
{
	playerInterface.setUnitCostInfo(unitCostInfo);
}

void Renderer::setPickupTexture(sf::Texture* textureToSet)
{
	pickupTexture = textureToSet;
}

float Renderer::getMenuHeight()
{
	return playerInterface.getMenuHeight();
}

void Renderer::setInfoBarSprite(sf::Texture* textureToSet, float squareSize, float infoBarHeight)
{
	playerInterface.setInfoBarSprite(textureToSet, windowSizeX, windowSizeY, squareSize, infoBarHeight);
}

float Renderer::resizeWindow(float x, float y, Game *game)
{
	windowSizeX = x / squareSize;
	windowSizeY = y / squareSize;
	zoom = 1;
	
	mainView.setSize(x, y);
	
	mainView.setCenter(x / 2, y / 2);
	mainView.zoom(zoom);

	bValueX = -0.5 * windowSizeX;
	cValueX = 0.5 * windowSizeX;
	bValueY = -0.5 * windowSizeY;
	cValueY = 0.5 * windowSizeY;

	windowPosX = 0;
	windowPosY = 0;
	windowScrollOffset[0] = 0;
	windowScrollOffset[1] = 0;
	HUD.setSize(x, y);
	HUD.setCenter(x / 2, y / 2);

	GameObject *selectedObject = NULL;

	if (game->currentMultiSelectedObjects.size() > 0)
		selectedObject = game->currentMultiSelectedObjects[0];

	return playerInterface.resizeHUD(game, 
		windowSizeX, 
		windowSizeY, 
		squareSize, 
		MENU_SIZE_X, 
		MENU_SIZE_Y, 
		game->mousePos.x, 
		game->mousePos.y, 
		game->playerResourceCount, 
		game->menuOptionSelected, 
		selectedObject, 
		game->UNIT_INFO_BOX_HEIGHT);
}

void Renderer::resetVision()
{
	//Reset to not visible
	for (int i = 0; i < worldSizeX; i++)
	{
		for (int j = 0; j < worldSizeY; j++)
		{
			//Check if was visible before
			//If was visible before last seen id will go to the object on it
			//If actually still visible will be sorted out farther down. If was invisible before, will retain the previously given last seen
			if (fogOfWar[i][j].getCurrentVisiblePlayer())
			{
				if (backgroundTiles[i][j].getOccupiedObj())
				{
					fogOfWar[i][j].setLastSeenIdPlayer(backgroundTiles[i][j].getOccupiedObj()->getID());
				}
				else
				{
					fogOfWar[i][j].setLastSeenIdPlayer(-1);
				}

				GameObject* focusObject = backgroundTiles[i][j].getOccupiedObj();

				//Find the object and see if its cord is this tile
				if (focusObject)
				{
					if (!focusObject->getIsPlayer())
					{
						if (focusObject->getGridPosX() == i && focusObject->getGridPosY() == j)
						{
							fogOfWar[i][j].setLastSeenCordOccupies(true);
						}
					}
				}
			}
			if (fogOfWar[i][j].getCurrentVisibleOpponent())
			{
				if (backgroundTiles[i][j].getOccupiedObj())
				{
					fogOfWar[i][j].setLastSeenIdOpponent(backgroundTiles[i][j].getOccupiedObj()->getID());
				}
				else
				{
					fogOfWar[i][j].setLastSeenIdOpponent(-1);
				}
			}

			fogOfWar[i][j].setCurrentVisiblePlayer(false);
			fogOfWar[i][j].setCurrentVisibleOpponent(false);

		}
	}

	//Go through all the game objects and enable vision accordingly
	for (int k = 0; k < playerInGameObjects.size(); k++)
	{
		if (playerInGameObjects.at(k)->getActive())
		{
			
			for (int i = -1 * playerInGameObjects.at(k)->getVision(); i <= playerInGameObjects.at(k)->getVision(); i++)
			{
				for (int j = -1 * playerInGameObjects.at(k)->getVision(); j <= playerInGameObjects.at(k)->getVision(); j++)
				{
					if (playerInGameObjects.at(k)->getPreviousPosX() + i < worldSizeX && playerInGameObjects.at(k)->getPreviousPosY() + j < worldSizeY && playerInGameObjects.at(k)->getPreviousPosX() + i >= 0 && playerInGameObjects.at(k)->getPreviousPosY() + j >= 0)
					{
						//This square is now visible. If the object last seen in this location is no longer active, it should be deleted because it no longer has a purpose
						if (fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].getLastSeenIdPlayer() >= 0)
						{
							GameObject* gottenObj = getGameObjectWithId(false, fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].getLastSeenIdPlayer());

							if (gottenObj && !gottenObj->getActive())
							{
								//Delete the object
								for (int ind = 0; ind < opponentInGameObjects.size(); ind++)
								{
									if (gottenObj->getID() == opponentInGameObjects[ind]->getID())
									{
										opponentInGameObjects.erase(opponentInGameObjects.begin() + ind);
										delete gottenObj;
									}
								}
								
							}
						}

						fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].setEnabled(false);
						fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].setCurrentVisiblePlayer(true);
						fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].setLastSeenCordOccupies(false);
						fogOfWar[playerInGameObjects.at(k)->getPreviousPosX() + i][playerInGameObjects.at(k)->getPreviousPosY() + j].setLastSeenIdPlayer(-1);

						
					}
				}
			}
			
		}
	}
	for (int k = 0; k < opponentInGameObjects.size(); k++)
	{
		if (opponentInGameObjects.at(k)->getActive())
		{
			
			
			for (int i = -1 * opponentInGameObjects.at(k)->getVision(); i <= opponentInGameObjects.at(k)->getVision(); i++)
			{
				for (int j = -1 * opponentInGameObjects.at(k)->getVision(); j <= opponentInGameObjects.at(k)->getVision(); j++)
				{
					if (opponentInGameObjects.at(k)->getPreviousPosX() + i < worldSizeX && opponentInGameObjects.at(k)->getPreviousPosY() + j < worldSizeY && opponentInGameObjects.at(k)->getPreviousPosX() + i >= 0 && opponentInGameObjects.at(k)->getPreviousPosY() + j >= 0)
					{
						if (fogOfWar[opponentInGameObjects.at(k)->getPreviousPosX() + i][opponentInGameObjects.at(k)->getPreviousPosY() + j].getLastSeenIdOpponent() >= 0)
						{
							GameObject* gottenObj = getGameObjectWithId(true, fogOfWar[opponentInGameObjects.at(k)->getPreviousPosX() + i][opponentInGameObjects.at(k)->getPreviousPosY() + j].getLastSeenIdOpponent());

							if (gottenObj && !gottenObj->getActive())
							{
								//Delete the object
								for (int ind = 0; ind < playerInGameObjects.size(); ind++)
								{
									if (gottenObj->getID() == playerInGameObjects[ind]->getID())
									{
										playerInGameObjects.erase(playerInGameObjects.begin() + ind);
										delete gottenObj;
									}
								}

							}
						}

						fogOfWar[opponentInGameObjects.at(k)->getPreviousPosX() + i][opponentInGameObjects.at(k)->getPreviousPosY() + j].setEnabledOpponent(false);
						fogOfWar[opponentInGameObjects.at(k)->getPreviousPosX() + i][opponentInGameObjects.at(k)->getPreviousPosY() + j].setCurrentVisibleOpponent(true);
						fogOfWar[opponentInGameObjects.at(k)->getPreviousPosX() + i][opponentInGameObjects.at(k)->getPreviousPosY() + j].setLastSeenIdOpponent(-1);
					}
				}
			}
			
		}
	}
}

void Renderer::giveBasesUnitResources(int* playerResources, int* opponentResources)
{
	for (int i = 0; i < playerInGameObjects.size(); i++)
	{
		if (playerInGameObjects.at(i)->getActive())
		{
			if (playerInGameObjects.at(i)->getPickupCarried())
			{
				//Check if its next to a base
				for (int j = 0; j < structureRefsPlayer.size(); j++)
				{
					if (structureRefsPlayer.at(j)->getActive()
						&& playerInGameObjects.at(i)->getGridPosX() >= structureRefsPlayer.at(j)->getGridPosX() - 1
						&& playerInGameObjects.at(i)->getGridPosX() <= structureRefsPlayer.at(j)->getGridPosX() + structureRefsPlayer.at(j)->getGridSizeX()
						&& playerInGameObjects.at(i)->getGridPosY() >= structureRefsPlayer.at(j)->getGridPosY() - 1
						&& playerInGameObjects.at(i)->getGridPosY() <= structureRefsPlayer.at(j)->getGridPosY() + structureRefsPlayer.at(j)->getGridSizeY())
					{
						//Add to total
						*playerResources += playerInGameObjects.at(i)->getPickupCarried()->getValue();
						delete playerInGameObjects.at(i)->getPickupCarried();
						playerInGameObjects.at(i)->setPickupCarried(nullptr);
						break;
					}
				}
			}
		}
	}

	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		if (opponentInGameObjects.at(i)->getActive())
		{
			if (opponentInGameObjects.at(i)->getPickupCarried())
			{
				//Check if its next to a base
				for (int j = 0; j < structureRefsOpponent.size(); j++)
				{
					if (structureRefsOpponent.at(j)->getActive()
						&& opponentInGameObjects.at(i)->getGridPosX() >= structureRefsOpponent.at(j)->getGridPosX() - 1
						&& opponentInGameObjects.at(i)->getGridPosX() <= structureRefsOpponent.at(j)->getGridPosX() + structureRefsOpponent.at(j)->getGridSizeX()
						&& opponentInGameObjects.at(i)->getGridPosY() >= structureRefsOpponent.at(j)->getGridPosY() - 1
						&& opponentInGameObjects.at(i)->getGridPosY() <= structureRefsOpponent.at(j)->getGridPosY() + structureRefsOpponent.at(j)->getGridSizeY())
					{
						//Add to total
						*opponentResources += opponentInGameObjects.at(i)->getPickupCarried()->getValue();
						delete opponentInGameObjects.at(i)->getPickupCarried();
						opponentInGameObjects.at(i)->setPickupCarried(nullptr);
						break;
					}
				}
			}
		}
	}
}

void Renderer::addTileToListOfResourceTiles(int x, int y)
{
	resourceTiles.push_back(&backgroundTiles[x][y]);
}

void Renderer::centerCamera(int gridX, int gridY)
{
	//Put this in the center
	gridX = gridX - (windowSizeX / 2);
	gridY = gridY - (windowSizeY / 2);

	float difX = (gridX * squareSize) - (windowPosX * squareSize + windowScrollOffset[0]);
	float difY = (gridY * squareSize) - (windowPosY * squareSize + windowScrollOffset[1]);

	windowScrollOffset[0] = 0;
	windowScrollOffset[1] = 0;

	windowPosX = (gridX);
	windowPosY = (gridY);

	mainView.move(difX, difY);
}

void Renderer::changeZoom(int newSquareSize)
{
	//Resize and reposition everything
}

GameObject* Renderer::getAgentBase(std::string type)
{
	//Should add it to the one that is the least busy
	GameObject* leastBusyBase = NULL;
	int minNumActions = -1;

	//All of them are busy now look for the 
	for (int i = 0; i < opponentInGameObjects.size(); i++)
	{
		if (opponentInGameObjects.at(i)->getCreatesType() == type)
		{
			if (minNumActions < 0 || opponentInGameObjects.at(i)->getActionQueueSize() < minNumActions)
			{
				minNumActions = opponentInGameObjects.at(i)->getActionQueueSize();
				leastBusyBase = opponentInGameObjects.at(i);
			}
		}
	}

	return leastBusyBase;
}

GameObject* Renderer::getGameObjectAgent(bool isPlayer, bool isStructure, std::string type, bool isBusy, bool isVisible)
{
	//Should add it to the one that is the least busy
	GameObject* leastBusy = NULL;
	int minNumActions = -1;

	std::vector<GameObject *> *structureRefs = &structureRefsOpponent;
	std::vector<GameObject *> *objectRefs = &opponentInGameObjects;

	if (isPlayer)
	{
		structureRefs = &structureRefsPlayer;
		objectRefs = &playerInGameObjects;
	}

	if (isStructure)
	{
		//All of them are busy now look for the 
		for (int i = 0; i < structureRefs->size(); i++)
		{
			if (structureRefs->at(i)->getActive() && structureRefs->at(i)->getType() == type)
			{
				if (isBusy && (!isVisible || Fog::unitSeen(
					fogOfWar, isPlayer, structureRefs->at(i)->getID(), structureRefs->at(i)->getGridPosX(), structureRefs->at(i)->getGridPosY())))
				{
					if (minNumActions < 0 || structureRefs->at(i)->getActionQueueSize() < minNumActions)
					{
						minNumActions = structureRefs->at(i)->getActionQueueSize();
						leastBusy = structureRefs->at(i);
					}
				}
				else if(!isVisible || Fog::unitSeen(
					fogOfWar, isPlayer, structureRefs->at(i)->getID(), structureRefs->at(i)->getGridPosX(), structureRefs->at(i)->getGridPosY()))
				{
					return structureRefs->at(i);
				}
			}
		}
	}
	else
	{
		//All of them are busy now look for the 
		for (int i = 0; i < objectRefs->size(); i++)
		{
			if (objectRefs->at(i)->getActive() && objectRefs->at(i)->getType() == type)
			{
				if (isBusy && (!isVisible || Fog::unitSeen(
					fogOfWar, isPlayer, objectRefs->at(i)->getID(), objectRefs->at(i)->getGridPosX(), objectRefs->at(i)->getGridPosY())))
				{
					if (minNumActions < 0 || objectRefs->at(i)->getActionQueueSize() < minNumActions)
					{
						minNumActions = objectRefs->at(i)->getActionQueueSize();
						leastBusy = objectRefs->at(i);
					}
				}
				else if(!isVisible || Fog::unitSeen(
					fogOfWar, isPlayer, objectRefs->at(i)->getID(), objectRefs->at(i)->getGridPosX(), objectRefs->at(i)->getGridPosY()))
				{
					return objectRefs->at(i);
				}
			}
		}

		if (leastBusy && leastBusy->busy())
			return NULL;
	}
	
	return leastBusy;
}

void Renderer::setMainBase(bool isPlayer, GameObject* mainBase)
{
	if (isPlayer)
	{
		playerBase = mainBase;
	}
	else
	{
		enemyBase = mainBase;
	}
}

void Renderer::giveTitleScreen(sf::Texture* textToSet)
{
	playerInterface.giveTItleScreenATexture(game, textToSet);
}