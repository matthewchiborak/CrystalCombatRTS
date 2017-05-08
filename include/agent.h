/*
	Agent Class (agent.h, agent.cpp)

	Created: Oct 21, 2016
	Author: Daniel Bailey

	Description:
		The agent files are used to define the core data
		structures and methods associated with the learning
		artificial intelligence agent.
*/

#ifndef __AGENT_H
#define __AGENT_H

/*
	Hard-limit for memory line input
	- Can be modified if required
*/
#define BUFFER_SIZE 50

// INCLUDES
// System Headers
#include <fstream>
#include <time.h>
#include <vector>

// Custom Headers
#include "gametypes.h"
#include "lock.h"
#include "logger.h"
#include "sleeper.h"
#include "game.h"

// HEURISTICS FOR AI AGENT
#define RESOURCE_PRIORITY 1
#define STRUCTURE_PRIORITY 2
#define UNIT_PRIORITY 3

// Struct for Agent Capabilities
struct Capability
{
	enum SubType 
	{
		None = -1,
		Unit = 0,
		Structure = 1,
		Area = 2
	};

	// Game action to perform
	Action action;

	// Specific sub-action info
	SubType subType;

	// Value thresholds
	int time, resource, unit, structure;

	// Default weight values
	float timeWeight, resourceWeight, unitWeight, structureWeight;

	// Calculated overall weight
	float weight;

	// Max allowed weight
	float maxWeight;

	Capability(const Action action, const Capability::SubType subType = Capability::SubType::None)
	{
		// Set action data
		this->subType = subType;
		this->action = action;
		// Default to max weights
		timeWeight = 0.f;
		resourceWeight = 0.f;
		unitWeight = 0.f;
		structureWeight = 0.f;
		maxWeight = 1.f;
		weight = 0.f;
		// Default to no scores
		time = 0;
		resource = 0;
		unit = 0;
		structure = 0;
	}
};

// FORWARD DECLARATIONS
class Agent;
class Brain;
class Memory;

// Agent entrypoint
void agent_begin(int *status, Lock *lock, Game *game);

// File thread
void agent_file(int *status, Lock *lock, Agent *agent);

// Decision making functions
static void populateCapabilities(std::vector<Capability> &capabilities);

/*
	Agent
	- This class is the artificial intelligence
		agent's core class. It provides the methods
		necessary for the game to access the stored
		agent data to modify the agent's brain.
*/
class Agent
{
private:
	// The agent's brain data
	Brain *_brain;
	// The game reference
	Game &game;
public:
	// INITIALIZERS

	// Default constructor
	Agent(Game &game);
	// Deconstructor
	~Agent();
	// Clear all brain data
	void clear();
	// Initialize the agent (wake up the agent)
	bool init();
	// Reloads the memory data
	bool reload();
	// Gets the brain data reference
	Brain *brain() { return _brain; };

};

/*
	Brain
	- This class is used to store all memory
		information for the current AI
*/
class Brain
{
private:
	// Pointer to the first Memory in the linked list
	Memory *_head;

	// How many memories are loaded
	int memoryCount;

	// Game reference
	Game &game;
public:
	// INITIALIZERS

	// Default constructor
	Brain(Game &game);
	// Deconstructor
	~Brain();

	// Add a memory into the memory list
	void addMemory(Memory *memory);

	// Gets first memory in list
	Memory *head() { return _head; };
};

/*
	Memory
	- Stores a specific memory of an event
		or outcome to be used in the 
		decision-making process
*/
class Memory
{
private:
	// Resource count for this memory
	score _resourceScore;
	// Relative unit strength
	score _unitScore;
	// Relative structure worth
	score _structureScore;
	// The action's score (Good or bad?)
	score _result;
	// Seconds into the game
	gametime _time;
	// What action this memory represents
	Action _action;

	// Next memory in linked list
	Memory *_next;
	// Previous memory in linked list
	Memory *_prev;

public:
	// INITIALIZERS

	// Default constructor
	Memory();
	// Deconstructor
	~Memory();

	// Initialize the variables
	void init(const score scores[4], const gametime time, const Action action);

	// Debug print memory contents
	void print(std::ostream &out);

	// Loads memory from file into brain
	static bool load(std::ifstream &file, Brain *brain);

	// Get the action
	const Action getAction() { return this->_action; };

	// Get links in list
	Memory *next() { return _next; };
	Memory *prev() { return _prev; };

	// Set links in list
	void setNext(Memory *next) { _next = next; };
	void setPrev(Memory *prev) { _prev = prev; };

	// Comparison - check all game variables are the same
	bool operator==(const Memory &other)
	{
		return (other._action == _action &&
			other._resourceScore == _resourceScore &&
			other._result == _result &&
			other._structureScore == _structureScore &&
			other._time == _time &&
			other._unitScore == _unitScore);
	}
};

#endif