/*
	Main entrypoint into entire application
	- thread manager
*/

#include "../include/agent.h"
#include "../include/game.h"

int main()
{
	// App status code
	int status = 1;

	// Shared game resource for both threads
	Game game;

	// Start the agent thread
	Logger::log(std::cout, "Launching agent thread");
	Lock agent_lock;
	std::thread agent_thread(agent_begin, &status, &agent_lock, &game);

	// Start the game thread
	Logger::log(std::cout, "Launching game thread");
	Lock game_lock;
	std::thread game_thread(game_begin, &status, &game_lock, &game);

	// Run the application in a loop
	while (status > 0); // 1 is running, 0 is exiting, < 0 is error

	// Done - wait and then join
	Logger::log(std::cout, "Closing threads...");
	agent_thread.join();
	game_thread.join();
	Logger::log(std::cout, "Done");

	// Close
	return status;
}