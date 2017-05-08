/*
	Sleeper Function Header (sleeper.h)

	Created: Jan 20, 2017
	Author: Daniel Bailey

	Description:
		This file provides sleep function wrappers.
*/

#ifndef __SLEEPER_H
#define __SLEEPER_H

#include <thread>
#include <chrono>

// Makes a thread sleep for a number of milliseconds
static void sleep_milli(const unsigned int milliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

// Makes a thread sleep for a number of seconds
static void sleep(const unsigned int seconds)
{
	std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

#endif