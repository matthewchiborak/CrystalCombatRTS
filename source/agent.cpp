/*
	Please see agent.h for documentation
*/

//Include the agent header file
#include "../include/agent.h"

//TODO include custom data structure types

/*
	Public Function Definitions
*/

// AGENT CLASS

// The default constructor
Agent::Agent(Game &game) : game(game)
{
	// Initialize variables
	_brain = NULL;
}

// The deconstructor - called during an expected shutdown
Agent::~Agent()
{
	// Clear any associated data from memory
	clear();
}

/*
	Agent::clear()
	- clears all brain data
*/
void Agent::clear()
{
	if (_brain)
		delete _brain;
	_brain = NULL;
}

/*
	Agent::init()
	- initializes/loads the agent
*/
bool Agent::init()
{
	// Initialize the brain data structure
	if (_brain)
		delete _brain;
	_brain = new Brain(game);

	return reload();
}

/*
	Agent::reload()
	- Reloads memory data to the agent
*/
bool Agent::reload()
{
	// File to load the data
	std::ifstream brainFile;

	// Any file accesses must be exception handled
	try
	{
		/*
			Open the core brain file
			- the brain file contains
				information that points
				to each specific stream
				of memory data
		*/
		brainFile.open("resources/agent/brain.dat");

		// First make sure the file opened successfully		
		if (brainFile.is_open() && brainFile.good())
		{
			// Variable to store the input data
			char input[BUFFER_SIZE];

			// Now check each line for bank data
			while (brainFile.is_open() && brainFile.good() && !brainFile.eof())
			{
				// Now load each memory
				if (!Memory::load(brainFile, _brain))
				{
					throw std::exception("Memory failed to load");
				}
			}
		}
		else
		{
			// File failed to load - throw an exception for this
			throw std::exception("brain.dat failed to load. The file may not exist.");
		}

		// All done - close the file
		brainFile.close();

		// Return a success boolean
		return true;
	}
	// If an exception is thrown, we gracefully handle it
	catch (std::exception e)
	{
		// First close the file
		brainFile.close();

		// Now output the exception's associated message
		Logger::log_error(std::cout, e.what());

		// Return a failure boolean
		return false;
	}
}

// BRAIN CLASS

// The default constructor
Brain::Brain(Game &game) : game(game)
{
	//The linked list has no head to start
	_head = NULL;
	memoryCount = 0;
}

// The deconstructor
Brain::~Brain()
{
	//I there is associated memory data, delete it
	while (_head)
	{
		if (_head->next())
		{
			_head = _head->next();
			delete _head->prev();
		}
		else
		{
			delete _head;
			_head = NULL;
		}

		memoryCount--;

		Logger::log(std::cout, "Count: " + std::to_string(memoryCount));
	}
}

void Brain::addMemory(Memory *memory)
{
	if (!_head)
	{
		_head = memory;
		memoryCount = 1;
	}
	else
	{
		// Pointer to linked list iterator
		Memory *current = _head;

		// Loop to end of list
		while (current->next() != NULL)
		{
			// Ensure not a duplicate
			if ((*current) == (*memory))
			{
				return; // Do not add same memory
			}
			// Keep traversing
			current = current->next();
		}
		// Ensure not a duplicate
		if ((*current) == (*memory))
		{
			return; // Do not add same memory
		}
		// At end of list
		current->setNext(memory);
		memory->setPrev(current);
		memoryCount++;
	}

	Logger::log(std::cout, "Memory " + std::to_string(memoryCount));
	memory->print(std::cout);
}

// MEMORY CLASS

// The default constructor
Memory::Memory()
{
	// Initialize the empty memory object
	_next = NULL;
	_prev = NULL;
}

// The deconstructor
Memory::~Memory()
{
	Logger::log(std::cout, "Deleting memory");
}

void Memory::init(const score scores[4], const gametime time, const Action action)
{
	// Initialize our data
	_resourceScore = scores[0];
	_unitScore = scores[1];
	_structureScore = scores[2];
	_result = scores[3];
	_time = time;
	_action = action;
}

void Memory::print(std::ostream &out)
{
	out << "Memory: \n";
	out << "  Resource Score: " << _resourceScore << "\n";
	out << "      Unit Score: " << _unitScore << "\n";
	out << " Structure Score: " << _structureScore << "\n";
	out << "          Result: " << _result << "\n";
	out << "       Game Time: " << _time << " seconds\n";
	out << "       Action ID: " << (score)_action << "\n";
}

bool Memory::load(std::ifstream &file, Brain *brain)
{
	if (!file.good() || file.eof())
		return false;

	// Our memory object
	Memory *memory = new Memory();

	// Load the data for the memory
	score temp_score[4];
	gametime temp_time;
	score temp_action;

	for (char i = 0; i < 4; i++)
	{
		if (file.good() && !file.eof())
			file >> temp_score[i];
		else
			return false;
	}

	if (file.good() && !file.eof())
		file >> temp_time;
	else
		return false;

	if (file.good() && !file.eof())
		file >> temp_action;
	else
		return false;

	// Init the memory object
	memory->init(temp_score, temp_time, Action(temp_action));

	// Insert into brain
	brain->addMemory(memory);

	return true;
}
