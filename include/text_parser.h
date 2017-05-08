/*
TextParser Class (text_parser.h, text_parser.cpp)

Created: Jan 20ish, 2017
Author: Matthew Chiborak

Description:
Class that is used in order to read and process text files in order to get information about the game and loading things like textures etc. 
*/

#ifndef __TEXTPARSER_H
#define __TEXTPARSER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class TextParser
{
	public:
		TextParser();
		~TextParser();

		std::string loadTextFile(std::string location);

		//Parse the specified text file and split it up by next line
		std::vector<std::string>* parseTextFileByLine(std::string location);

		//Tired of typing out std::cout << etc. Well here ya go.
		static void print(std::string stringToPrint);

		//Split a string by a specified character
		std::vector<std::string>* splitString(std::string stringToSplit, char delimiter);


};


#endif

