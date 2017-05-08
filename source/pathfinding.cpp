#include "../include/pathfinding.h"
#include "../include/Renderer.h"
#include "../include/sleeper.h"

PathMap::PathMap() 
{
	// Initialize the path map to null
	costs = NULL;
	worldSizeX = 0;
	worldSizeY = 0;
}

PathMap::~PathMap() 
{
	// If there is data, remove it
	if (costs)
	{
		for (int x = 0; x < worldSizeX; x++)
		{
			delete[] costs[x];
		}

		delete[] costs;
	}
}

PathMap * PathMap::getPathMap()
{
	if (!s_instance)
		s_instance = new PathMap();
	return s_instance;
}

bool PathMap::tryLock()
{
	return lock.try_lock();
}

void PathMap::unlock()
{
	lock.unlock();
}

const int PathMap::WorldSizeX()
{
	return worldSizeX;
}

const int PathMap::WorldSizeY()
{
	return worldSizeY;
}

const int PathMap::Cost(const int x, const int y)
{
	if (!costs || x < 0 || y < 0 || x >= worldSizeX || y >= worldSizeY)
		return -1;
	else
		return costs[x][y];
}

void PathMap::init(Renderer & renderer)
{
	// Store map size
	worldSizeX = renderer.getWorldSizeX();
	worldSizeY = renderer.getWorldSizeY();

	// Store tile costs
	costs = new int*[worldSizeX];
	for (int x = 0; x < worldSizeX; x++)
	{
		costs[x] = new int[worldSizeY];
		for (int y = 0; y < worldSizeY; y++)
		{
			// Flying units can go over walls
			costs[x][y] = renderer.getTile(x, y)->getCost();
		}
	}
}

void PathMap::setCost(const int x, const int y, const int cost)
{
	if (x < 0 || x >= worldSizeX ||
		y < 0 || y >= worldSizeY)
		return;
	costs[x][y] = cost;
}

Pathfinder::Pathfinder(const int startX, const int startY, const int endX, const int endY, std::mutex *lock, const long long startTime, const bool flying)
{
	// Launch the pathfinding thread
	path_thread = new std::thread(pathfind_main, this, lock, flying, startTime, startX, startY, endX, endY);
}

Pathfinder::~Pathfinder()
{
	if (path_thread)
	{
		path_thread->join();
		delete path_thread;
	}
}

void Pathfinder::cancel()
{
	_cancelled = true;
}

const bool Pathfinder::cancelled()
{
	return _cancelled;
}

void Pathfinder::finish()
{
	_finished = true;
}

const bool Pathfinder::finished()
{
	return _finished;
}

void Pathfinder::initMap(PathMap & pathmap, const bool flying)
{
	// Temp vars for initializing tiles
	int cost;
	std::string id;

	// Tile loop
	_map = new Tile*[_mapSizeX];
	for (int x = 0; x < _mapSizeX; x++)
	{
		_map[x] = new Tile[_mapSizeY];

		for (int y = 0; y < _mapSizeY; y++)
		{
			// Flying units can go over anything
			if (flying)
				cost = DEFAULT_TILE_COST;
			else
				cost = pathmap.Cost(x, y);
			// Set the id and initialize the map
			id = std::to_string(x) + std::to_string(y);
			_map[x][y].init(cost, x, y, id);
		}
	}

}

void Pathfinder::reset(const long long setStartTime)
{
	_finished = false;
	_cancelled = false;
	_startTime = setStartTime;
}

const long long Pathfinder::startTime()
{
	return _startTime;
}

void Pathfinder::setStartPos(Tile * newStartPos)
{
	_startPos = newStartPos;
}

void Pathfinder::setEndPos(Tile * newEndPos)
{
	_endPos = newEndPos;
}

void Pathfinder::setWorldSizeX(const int worldSizeX)
{
	_mapSizeX = worldSizeX;
}

void Pathfinder::setWorldSizeY(const int worldSizeY)
{
	_mapSizeY = worldSizeY;
}
Tile* Pathfinder::startPos()
{
	return _startPos;
}

Tile* Pathfinder::endPos()
{
	return _endPos;
}

Tile** Pathfinder::map()
{
	return _map;
}

const int Pathfinder::mapSizeX()
{
	return _mapSizeX;
}

const int Pathfinder::mapSizeY()
{
	return _mapSizeY;
}

TileQueue& Pathfinder::frontier()
{
	return _frontier;
}

std::vector<Tile*>& Pathfinder::explored()
{
	return _explored;
}

Tile::Tile()
{
	_cost = 0;
	_totalCost = -1;
	_priority = -1;
	_xPos = -1;
	_yPos = -1;
	_id = "";
	_parent = NULL;
}

void Tile::init(int cost, int xPos, int yPos, std::string id)
{
	_cost = cost;
	_xPos = xPos;

	_yPos = yPos;
	_id = std::to_string(xPos) + std::to_string(yPos);
}

void Tile::setTotalCost(const int totalCost)
{
	_totalCost = totalCost;
}

const int Tile::getTotalCost()
{
	return _totalCost;
}

void Tile::setCost(const int cost)
{
	_cost = cost;
}

const int Tile::getCost()
{
	return _cost;
}

void Tile::setParent(Tile * parent)
{
	_parent = parent;
}

Tile * Tile::getParent()
{
	return _parent;
}

void Tile::setPriority(const int priority)
{
	_priority = priority;
}

const int Tile::getPriority()
{
	return _priority;
}

const std::string Tile::getID()
{
	return _id;
}

const int Tile::getX()
{
	return _xPos;
}

const int Tile::getY()
{
	return _yPos;
}

/*
	Pathfinding thread
*/
void pathfind_main(Pathfinder* pathfinder, std::mutex *lock, const bool flying, const long long startTime, const int startX, const int startY, const int endX, const int endY)
{
	// Get lock
	while (!lock->try_lock());

	// Initialize variables
	pathfinder->reset(startTime);

	// For init, get pathfind lock
	PathMap *pathmap = PathMap::getPathMap();
	while (!pathmap->tryLock());

	// Is this a valid path?
	pathfinder->setWorldSizeX(pathmap->WorldSizeX());
	pathfinder->setWorldSizeY(pathmap->WorldSizeY());
	if (startX < 0 || startY < 0 || endX < 0 || endY < 0 ||
		startX >= pathfinder->mapSizeX() || startY >= pathfinder->mapSizeY() ||
		endX >= pathfinder->mapSizeX() || startY >= pathfinder->mapSizeY())
	{
		pathmap->unlock();
		pathfinder->cancel();
		return;
	}

	// Temp id var for tiles
	std::string id;
	int cost = 0;

	// Create the map
	pathfinder->initMap(*pathmap, flying);

	// Set start and end tiles
	pathfinder->setStartPos(&pathfinder->map()[startX][startY]);
	pathfinder->setEndPos(&pathfinder->map()[endX][endY]);

	// No need for pathmap anymore
	pathmap->unlock();
    
	// Variables
	Tile *currentNode = NULL;
	Tile *nextNode = NULL;
	int x, y, dx, dy;
	int nextCost, priority;
	bool diagonal, explored, wall;

	// Initialize the a* algorithm
	Pathfinder &p = (*pathfinder);
	p.startPos()->setTotalCost(0);
	p.startPos()->setPriority(heuristic(*p.startPos(), p, dx, dy));
	p.frontier().push(p.startPos());
	p.explored().push_back(p.startPos());

	// Pathfinding loop
	while(p.frontier().peek() && !p.cancelled())
    {
		// Get next node to explore
		currentNode = p.frontier().pop();

		// Are we at a goal state?
		if (currentNode->getID() == p.endPos()->getID())
		{
			// We have finished
			p.finish();
			break;
		}

		// Expand search
		for (x = currentNode->getX() - 1; x <= currentNode->getX() + 1; x++)
		{
			if(x >= 0 && x < p.mapSizeX())
			{
				for (y = currentNode->getY() - 1; y <= currentNode->getY() + 1; y++)
				{
					if (y >= 0 && y < p.mapSizeY() &&
						!(x == currentNode->getX() && y == currentNode->getY()))
					{
						// Is this diagonal to this tile?
						diagonal = (x != currentNode->getX() && y != currentNode->getY());

						// Get next node pointer
						nextNode = &p.map()[x][y];

						// Calculate the new total cost for this node
						if (diagonal)
							nextCost = currentNode->getTotalCost() + ((nextNode->getCost() * 3) / 2);
						else
							nextCost = currentNode->getTotalCost() + nextNode->getCost();

						// Have we explored this tile yet?
						explored = std::find(p.explored().begin(), p.explored().end(), nextNode) != p.explored().end();

						// Is this a wall tile?
						wall = nextNode->getCost() < 0;

						// If not, does it require moving through a wall?
						if (!wall && diagonal)
						{
							// Check left
							if (x < currentNode->getX())
							{
								// Top left
								if (y < currentNode->getY())
									wall = (p.map()[x][y + 1].getCost() < 0 || p.map()[x + 1][y].getCost() < 0);
								else // Bottom left
									wall = (p.map()[x][y - 1].getCost() < 0 || p.map()[x + 1][y].getCost() < 0);
							}
							else // Check right
							{
								// Top right
								if (y < currentNode->getY())
									wall = (p.map()[x][y + 1].getCost() < 0 || p.map()[x - 1][y].getCost() < 0);
								else // Bottom right
									wall = (p.map()[x][y - 1].getCost() < 0 || p.map()[x - 1][y].getCost() < 0);
							}
						}

						// If we have not explored this tile or we have and this path is shorter
						if (!wall && // NOT A WALL TILE
								(!explored || // NOT EXPLORED
								(nextNode->getTotalCost() >= 0 &&  // IS EXPLORED AND HAS A TOTAL COST
									nextCost < nextNode->getTotalCost()))) // THE NEW COST IS LOWER
						{
							nextNode->setTotalCost(nextCost);
							priority = nextCost + heuristic(*nextNode, p, dx, dy);
							
							if (nextNode->getID() == p.endPos()->getID())
								priority = -1; // MAX PRIORITY
							
							nextNode->setParent(currentNode);
							nextNode->setPriority(priority);
							p.frontier().push(nextNode);

							if (!explored)
							{
								p.explored().push_back(nextNode);
							}
						}
					}
				}
			}
		}
    }

	// Quick sanity check
	if (!currentNode)
	{
		p.cancel();
	}
	else if (!p.finished())
	{
		// Can we choose closest path? - find best one in explored
		for (unsigned int i = 0; i < p.explored().size(); i++)
		{
			if (p.explored()[i]->getCost() >= 0 &&
				p.explored()[i]->getPriority() - p.explored()[i]->getTotalCost() < 
					currentNode->getPriority() - currentNode->getTotalCost())
				currentNode = p.explored().at(i);
		}

		// If there is no gain from moving, don't - cancel the move
		if (heuristic(*p.startPos(), p, dx, dy) <= currentNode->getPriority() - currentNode->getTotalCost())
		{
			p.setEndPos(currentNode);
			p.finish();
		}
		else if (currentNode->getCost() >= 0)
		{
			p.setEndPos(currentNode);
			p.finish();
		}
		else // Failed
		{
			p.cancel();
		}
	} 

	// All done
	lock->unlock();
}

int heuristic(Tile& node, Pathfinder& pathfinder, int& dx, int& dy)
{
	// Compute manhattan distance
	dx = abs(node.getX() - pathfinder.endPos()->getX());
	dy = abs(node.getY() - pathfinder.endPos()->getY());
	return HEURISTIC_VALUE * (dx + dy);
}

TileQueue::TileQueue()
{
	head = NULL;
}

TileQueue::~TileQueue()
{
	// Clear node objects
	Node *iterator = head;
	while (head)
	{
		head = head->next;
		delete iterator;
		iterator = head;
	}
}

Tile* TileQueue::peek()
{
	if (head)
		return head->data;
	else
		return NULL;
}

void TileQueue::push(Tile * tile)
{
	// First push
	if (!head)
	{
		head = new Node(tile);
		return;
	}

	// Previous is used to update next references
	Node *prev = NULL;

	// Current is the traversal iterator
	Node *current = head;

	// If we are replacing the head, we need to set the new value of head
	bool isHead = false;

	// While we are iterating...
	while (current)
	{
		// Check priority conditions
		if (tile->getPriority() < current->data->getPriority())
		{
			// Are we at the head?
			isHead = (current == head);
			Node *ref = current;

			// Replace with new node
			current = new Node(tile);

			// Update next references
			current->next = ref;
			if (prev)
				prev->next = current;

			// This is the new head
			if (isHead)
				head = current;
			return;
		}
		
		// Iterate to next item
		if (current->next)
		{
			prev = current;
			current = current->next;
		}
		else
			break;
	}

	// Add to end of list
	current->next = new Node(tile);
}

Tile* TileQueue::pop()
{
	// No items, return null
	if(!head)
		return NULL;

	// We hold the data reference while we delete the entry
	Tile* data = head->data;

	// Store a ref to allow deletion
	Node* ref = head;

	// Move the head down to the next item
	head = head->next;

	// Delete this popped entry
	delete ref;

	// Return the data
	return data;
}

TileQueue::Node::Node()
{
	data = NULL;
	next = NULL;
}
TileQueue::Node::Node(Tile *data)
{
	this->data = data;
	next = NULL;
}

PathingList::PathingList()
{
	_head = NULL;
}

PathingList::~PathingList()
{
	clear();
}

void PathingList::add(GameObject * object)
{
	if (!object)
		return;

	if (!_head)
	{
		_head = new ListNode(object);
		return;
	}

	ListNode *node = _head;
	while (node->next)
	{
		// No repeats
		if (node->object == object)
			return;
		node = node->next;
	}

	// Add object to end
	node->next = new ListNode(object);
}

void PathingList::clear()
{
	ListNode *node = _head;
	while (node != NULL)
	{
		_head = node->next;
		delete node;
		node = _head;
	}
	_head = NULL;
}

PathingList::ListNode * PathingList::head()
{
	return _head;
}

void PathingList::remove(GameObject * object)
{
	if (!object || !_head)
		return;

	if (_head->object == object)
	{
		ListNode *newhead = _head->next;
		delete _head;
		_head = newhead;
		return;
	}

	ListNode *node = _head;
	while (node->next)
	{
		// Remove object
		if (node->next->object == object)
		{
			ListNode *del = node->next;
			node->next = del->next;
			delete del;
			return;
		}
		node = node->next;
	}
}
