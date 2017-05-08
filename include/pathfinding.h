/*
	Pathfinding Class (pathfinding.h, pathfinding.cpp)

	Created: Jan 23, 2017
	Author: Daniel Bailey

	Description:
	These files define the pathfinding algorithms and
	functions to call them from the logic thread.
*/

#ifndef __PATHFINDING_H
#define __PATHFINDING_H

#include <thread>
#include <vector>
#include <mutex>

#define HEURISTIC_VALUE 10

class PathMap;
class Pathfinder;
class Tile;
class TileQueue;
class Renderer;
class GameObject;

// The algorithm thread
void pathfind_main(Pathfinder *pathfinder, std::mutex *lock, const bool flying, const long long startTime, const int startX, const int startY, const int endX, const int endY);

// Our heuristic function 
int heuristic(Tile &node, Pathfinder &pathfinder, int& dx, int& dy);

class PathMap
{
private:
	static PathMap* s_instance;
	std::mutex lock;

	// Renderer data
	int worldSizeX;
	int worldSizeY;

	// Tile costs
	int **costs;

	PathMap();

public:
	~PathMap();

	// Funcions
	void init(Renderer &renderer);
	void setCost(const int x, const int y, const int cost);
	static PathMap* getPathMap();

	// Locking
	bool tryLock();
	void unlock();

	// Getters and setters
	const int WorldSizeX();
	const int WorldSizeY();
	const int Cost(const int x, const int y);
};

/*
	List of GameObjects that are
	pathfinding
*/
class PathingList
{
public:
	class ListNode
	{
	public:
		GameObject *object;
		ListNode *next;

		ListNode(GameObject *object)
		{
			this->object = object;
			next = NULL;
		}
	};
private:
	ListNode *_head;
public:
	PathingList();
	~PathingList();

	void add(GameObject *object);
	void clear();
	ListNode *head();
	void remove(GameObject *object);
};

/*
	The priority queue of tiles in
	the pathfinder
*/
class TileQueue
{
private:
	struct Node
	{
		Tile *data;
		Node *next;

		Node();
		Node(Tile *data);
	};
private:
	Node *head;
public:
	TileQueue();
	~TileQueue();

	Tile *peek();
	void push(Tile *tile);
	Tile *pop();
};

/*
	The Pathfinder class organizes each call to
	pathfind for objects. It allocates thread
	resources and calls the algorithms.
*/
class Pathfinder
{
private:
	// Have we found a path?
	bool _finished;
	// Bool to cancel
	bool _cancelled;
	// When did we initiate pathfinding?
	long long _startTime;
	// Our pathfinding thread
	std::thread* path_thread;
	// Where are we starting and going
	Tile* _startPos;
	Tile* _endPos;
	Tile** _map;
	int _mapSizeX, _mapSizeY;
	// Exploration area
	TileQueue _frontier;
	// The explored tiles
	std::vector<Tile*> _explored;
public:
	Pathfinder(const int startX, const int startY, const int endX, const int endY, std::mutex *lock, long long startTime, const bool flying);
	~Pathfinder();

	void cancel();
	const bool cancelled();

	void finish();
	const bool finished();

	void initMap(PathMap &pathmap, const bool flying);

	void reset(const long long setStartTime);
	
	const long long startTime();

	void setStartPos(Tile *newStartPos);
	void setEndPos(Tile *newEndPos);
	void setWorldSizeX(const int worldSizeX);
	void setWorldSizeY(const int worldSizeY);

	Tile *startPos();
	Tile *endPos();
	Tile **map();

	const int mapSizeX();
	const int mapSizeY();

	TileQueue& frontier();
	std::vector<Tile*>& explored();
};

class Tile
{
private:
	// Cost to move to this tile
	int _cost;
	// Cost to move from start to this tile
	int _totalCost;
	// Priority in the frontier
	int _priority;
	// Coordinates in tile array
	int _xPos, _yPos;
	// Identifier in hashmap
	std::string _id;
	// Parent in shortest path
	Tile *_parent;
public:
	Tile();

	void init(int cost, int xPos, int yPos, std::string id = "");

	void setTotalCost(const int totalCost);
	const int getTotalCost();

	void setCost(const int cost);
	const int getCost();

	void setParent(Tile *parent);
	Tile *getParent();

	void setPriority(const int priority);
	const int getPriority();

	const std::string getID();

	const int getX();
	const int getY();
};

#endif