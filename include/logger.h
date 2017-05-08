/*
	Static Logging Functions (logger.h)

	Created: Nov, 2016
	Author: Daniel Bailey

	Description:
	The logging functions make it easy for anyone
	to log data to multiple locations with just one
	function call.
*/

#ifndef __LOGGER_H
#define __LOGGER_H

#include <iostream>
#include <string>

/*
	Logger
	- used to hold all static logging functions
*/
namespace Logger 
{
	/*
		Log
		- Will log output to the console and the log file
		- ex: Logger::log(std::cout, "Hello World");
	*/
	static void log(std::ostream &out, std::string content)
	{
		out << content << std::endl;
	}

	/*
		Log Error
		- Will log output to the console and the log file with
			an error tag at the beginning of the message
		- ex: Logger::log_error(std::cout, "File could not be read");
	*/
	static void log_error(std::ostream &out, std::string error)
	{
		log(out, "[ERROR] " + error);
	}

	/*
		Log Warning
		- Will log output to the console and add a warning to the
			beginning of the message
		- ex: Logger::log_warning(std::cout, "Running out of video memory");
	*/
	static void log_warning(std::ostream &out, std::string warning)
	{
		log(out, "[WARNING] " + warning);
	}
}
#endif // !__LOGGER_H
