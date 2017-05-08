#include "../include/agent.h"

/*
	Agent requires multiple threads
	- one thread, this one, to perform
		thread management
	- another thread for file I/O
	- multiple threads (dynamically allocated)
		to perform data processing
*/

void agent_begin(int *status, Lock *lock, Game *game_pointer)
{
	// Game reference
	Game &game = *game_pointer;

	// File thread
	std::thread* fileThread;

	// Agent object
	Agent agent(game);

	// Current thread id
	int proc_id = 1;

	// For inserting actions
	AgentAction *newAction;
	
	if (!agent.init())
		*status = -1;
	else
	{
		// Set up the thread objects
		fileThread = new std::thread(agent_file, status, lock, &agent);

		// Agent decision variables
		int elapsedTime, resourceCount, unitScore, structureScore;
		int idleIndex;
		int bestIndex = 0;
		float bestWeight = 0.f;

		/*	MAP QUADRANTS - for scouting
			1 | 2
			-----
			3 | 4
		*/
		bool quadSet = false;
		int minXWall, midXWall, maxXWall;
		int minYWall, midYWall, maxYWall;

		// Our agent capabilities
		std::vector<Capability> capabilities;
		populateCapabilities(capabilities);

		// Object pointer
		GameObject *object = NULL;

		// Action adding
		AgentAction *actionToAdd = NULL;

		// Get index of idle task
		for (int i = 0; i < capabilities.size(); i++)
		{
			if (capabilities[i].action == Action::Idle)
			{
				idleIndex = i;
				break;
			}
		}

		// learnCapabilities(capabilities);

		while (*status > 0)
		{
			try
			{
				// Main decision making loop
				if (game.ready && game.aiActions.empty() && !game.aiProcessing)
				{
					// Get current resource state data
					elapsedTime = time(0) - game.timeStartedGame;
					resourceCount = game.opponentResourceCount; // game.actualOpponent.resources;
					structureScore = game.actualOpponent.structures;
					unitScore = game.actualOpponent.units;

					// Set quads
					if (!quadSet)
					{
						quadSet = true;
						minXWall = 0;
						midXWall = game.renderer->getWorldSizeX() / 2;
						maxXWall = game.renderer->getWorldSizeX() - 1;
						minYWall = 0;
						midYWall = game.renderer->getWorldSizeY() / 2;
						maxYWall = game.renderer->getWorldSizeY() - 1;
					}

					// Snap to 0
					if (structureScore < 0)
						structureScore = 0;
					if (unitScore < 0)
						unitScore = 0;

					// Assign weights for this round
					// Reset last best
					bestIndex = idleIndex;
					bestWeight = 0.f;

					// Weight each capability for this round
					for (int i = 0; i < capabilities.size(); i++)
					{
						Capability &c = capabilities[i];
						
						// Reset current weight
						c.weight = 0.f;

						// Factor in current data and learned data for each resource
						// Minimum time to start action
						if (elapsedTime >= c.time)
							c.weight += c.timeWeight;
						else
							c.weight += c.timeWeight * (elapsedTime / c.time);
						// Resources required for action
						if (resourceCount >= c.resource)
							c.weight += c.resourceWeight;
						else
							c.weight += c.resourceWeight * (resourceCount / c.resource);

						// Scores are relative to action - for building this is a max
						if (c.action == Action::Build)
						{
							// Unit score required for action 
							if (unitScore < c.unit)
								c.weight += c.unitWeight;

							// Structure score required for action
							if (structureScore < c.structure)
								c.weight += c.structureWeight;

							// Do we have the capability to perform this?
							if (c.subType == Capability::Structure)
							{
								if (!game.getObjectSpawner("base"))
									c.weight = 0.f;
							}
							else // Unit
							{
								if (!game.getObjectSpawner("wall") && !game.getObjectSpawner("worker"))
									c.weight = 0.f;
							}
						}
						else
						{
							// Unit score required for action 
							if (unitScore >= c.unit)
								c.weight += c.unitWeight;
							else
								c.weight += c.unitWeight * (unitScore / c.unit);
							// Structure score required for action
							if (structureScore >= c.structure)
								c.weight += c.structureWeight;
							else
								c.weight += c.structureWeight * (structureScore / c.structure);
						}

						if (c.action == Action::Attack)
						{
							// Are there viable targets?
							if (c.subType == Capability::SubType::Unit)
							{
								// If there is a valid unit to test
								if (!game.getGameObject(true, false, "test", false, true) &&
									!game.getGameObject(true, false, "wall", false, true))
									c.weight = 0;


							}
							else
							{
								if (!game.getGameObject(true, true, "base", false, true))
									c.weight = 0;
							}
						}

						if (c.weight > c.maxWeight)
							c.weight = c.maxWeight;

						if (i != idleIndex && c.weight >= bestWeight)
						{
							if (c.weight == bestWeight)
							{
								// Tie, random chance
								if (rand() % 2 == 0)
								{
									bestIndex = i;
									bestWeight = c.weight;
								}
							}
							else
							{
								bestIndex = i;
								bestWeight = c.weight;
							}
							
						}

					}

					// Add to action queue
					actionToAdd = new AgentAction();
					actionToAdd->type = capabilities[bestIndex].action;
					
					// TODO - query game for id of structure/unit matching parameters
					if (actionToAdd->type == Action::Build)
					{
						object = game.getObjectSpawner("worker");
						if (object)
						{
							actionToAdd->ids.push_back(object->getID()); // Base
							// TODO have easy interface for selecting unit to create
							actionToAdd->buildMenu = Menu::base;
							actionToAdd->buildOption = 3;
						}
						else
						{
							actionToAdd->type == Action::Idle;
						}
					}
					else if (actionToAdd->type != Action::Idle)
					{
						// Try each type of unit
						object = game.getGameObject(false, false, "test", true, false);
						if(!object)
							object = game.getGameObject(false, false, "wall", true, false);
						
						if (object)
						{
							actionToAdd->ids.push_back(object->getID()); // Standard Unit

							// Attacking - find target
							if (actionToAdd->type == Action::Attack)
							{
								if (capabilities[bestIndex].subType == Capability::SubType::Unit)
								{
									object = game.getGameObject(true, false, "test", false, true);
									if(!object)
										game.getGameObject(true, false, "wall", false, true);
									if (object)
										actionToAdd->target = object->getID();
								}
								else // Structure
								{
									object = game.getGameObject(true, true, "base", false, true);
									if (object)
										actionToAdd->target = object->getID();
								}
								// TODO add area attack
							}
							else if (!object->busy() && actionToAdd->type == Action::Scout)
							{
								// Since we are moving object - find a quadrant to move it to
								if (object->getGridPosX() < midXWall)
								{
									if (object->getGridPosY() < midYWall)
									{
										// Quad 1 -> Quad 4
										actionToAdd->targetTile.x = rand() % maxXWall + midXWall;
										actionToAdd->targetTile.y = rand() % maxYWall + midYWall;
									}
									else
									{
										// Quad 3 -> Quad 2
										actionToAdd->targetTile.x = rand() % maxXWall + midXWall;
										actionToAdd->targetTile.y = rand() % midYWall;
									}
								}
								else
								{
									if (object->getGridPosY() < midYWall)
									{
										// Quad 2 -> Quad 3
										actionToAdd->targetTile.x = rand() % midXWall;
										actionToAdd->targetTile.y = rand() % maxYWall + midYWall;
									}
									else
									{
										// Quad 4 -> Quad 1
										actionToAdd->targetTile.x = rand() % midXWall;
										actionToAdd->targetTile.y = rand() % midYWall;
									}
								}

								std::cout << "Moving " << object->getID() << " to [" <<
									actionToAdd->targetTile.x << ", " << actionToAdd->targetTile.y << "]" << std::endl;
							}
							else
							{
								actionToAdd->type == Action::Idle;
							}
							// TODO add farming in
						}
						else
						{
							actionToAdd->type == Action::Idle;
						}
					}

					// Add to game
					if (actionToAdd->type != Action::Idle)
					{
						Logger::log(std::cout, "Action to perform: " +
							std::to_string(bestIndex) + "with weight = " + floatToString(bestWeight, 3));
						game.aiActions.push(actionToAdd);
					}
					else
						delete actionToAdd;
					actionToAdd = NULL;
				}
			}
			catch (...)
			{
				Logger::log_error(std::cout, "Unhandled Exception");
			}

			sleep_milli(100);
		}

		// Close app, status is 0 - closing
		*status = 0;

		// Join and clean up threads
		fileThread->join();
		delete fileThread;
	}
}

void agent_file(int *status, Lock *lock, Agent *agent)
{
	// Timestamp
	time_t t;

	while (*status > 0)
	{
		// Run file loop
		try
		{
			// Threadsafe lock
			if (lock->lock())
			{
				time(&t);

				if (!agent->reload())
				{
					// Failed to load - clear and try again
					Logger::log_warning(std::cout, "Agent failed to reload. Trying again in 5 seconds...");
					sleep_milli(5000);

					if (!agent->init())
					{
						// Failed again - throw error
						throw std::exception("Agent cannot load memory data");
					}
				}

				// Return the lock
				lock->unlock();

				// Hang thread for a bit
				sleep_milli(2000);
			}
		}
		catch (...)
		{
			// If any exception occurs we need to modify the status bit
			*status = -1;
			Logger::log_error(std::cout, "Thread error. Exiting...");
			lock->unlock();
			return;
		}
	}
}

static void populateCapabilities(std::vector<Capability> &capabilities)
{
	capabilities.clear();
	
	// Idle
	Capability idle(Action::Idle);
	
	// Farm
	Capability farm(Action::Farm);
	//farm.resourceWeight = 0.25f;
	//farm.unitWeight = 0.5f;
	//farm.structureWeight = 0.25f;
	//farm.timeWeight = 0.f;

	// Scout
	Capability scout(Action::Scout);
	scout.resourceWeight = 0.25f;
	scout.timeWeight = 0.5f;
	scout.unitWeight = 0.25f;
	scout.structureWeight = 0.f;
	scout.maxWeight = 0.6f;
	scout.time = 0; // Scout right away
	scout.resource = 0; // Does not require resources
	scout.unit = 5; // Temp values

	// Build unit
	Capability buildUnit(Action::Build, Capability::SubType::Unit);
	buildUnit.resourceWeight = 0.25f;
	buildUnit.timeWeight = 0.125f;
	buildUnit.unitWeight = 0.5f;
	buildUnit.structureWeight = 0.125f;
	buildUnit.time = 0; // Can build right away
	buildUnit.unit = 5; // Build if has less than five
	buildUnit.resource = 200; // Build if has above 200
	buildUnit.structure = 0; // Temp value

	// Build structure
	Capability buildStructure(Action::Build, Capability::SubType::Structure);
	buildStructure.timeWeight = 0.25f;
	buildStructure.resourceWeight = 0.375f;
	buildStructure.unitWeight = 0.125f;
	buildStructure.structureWeight = 0.25f;;
	buildStructure.time = 15; // Waits a bit to build
	buildStructure.unit = 2; // Needs at least some defence
	buildStructure.resource = 500; // Build if has above 500
	buildStructure.structure = 0; // Temp value

	// Attack area
	Capability attackArea(Action::Attack, Capability::SubType::Area);
	attackArea.timeWeight = 0.25f;
	attackArea.unitWeight = 0.5f;
	attackArea.structureWeight = 0.125f;
	attackArea.resourceWeight = 0.125f;
	attackArea.maxWeight = 0.f;
	attackArea.time = 180; // 3 min
	attackArea.unit = 5; // 5 units
	attackArea.structure = 0; // Temp value - will need structures to replenish losses

	// Attack unit
	Capability attackUnit(Action::Attack, Capability::SubType::Unit);
	attackUnit.timeWeight = 0.f;
	attackUnit.unitWeight = 0.75f;
	attackUnit.structureWeight = 0.25f;
	attackUnit.resourceWeight = 0.f;
	attackUnit.unit = 1; // Needs a unit to assign the attack to
	attackUnit.structure = 2;

	// Attack structure
	Capability attackStructure(Action::Attack, Capability::SubType::Structure);
	attackStructure.timeWeight = 0.375f;
	attackStructure.unitWeight = 0.5f;
	attackStructure.structureWeight = 0.125f;
	attackStructure.resourceWeight = 0.f;
	attackStructure.time = 60; // Wait at least a minute before attacking structures
	attackStructure.unit = 5; // Needs a small force
	attackStructure.structure = 0; // Temp value - will need structures to replenish losses

	// Add more capabilities here

	// Push to the capability vector
	capabilities.push_back(idle);
	capabilities.push_back(farm);
	capabilities.push_back(scout);
	capabilities.push_back(buildUnit);
	capabilities.push_back(buildStructure);
	capabilities.push_back(attackArea);
	capabilities.push_back(attackUnit);
	capabilities.push_back(attackStructure);
}