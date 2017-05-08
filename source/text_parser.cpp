#include "../include/text_parser.h"



TextParser::TextParser()
{
}


TextParser::~TextParser()
{
}

std::string TextParser::loadTextFile(std::string location)
{
	std::string fileText = "";

	std::string line;
	std::ifstream myFile(location);

	try
	{
		if (myFile.is_open())
		{
			while (getline(myFile, line))
			{
				fileText += line;
				fileText += "\n";
			}

			myFile.close();
		}
		else
		{
			throw std::exception("Unable to open file\n");
		}
	}
	catch (std::exception e)
	{
		//Have the logger log the exception
		//Logger::log_error(std::cout, e.what());
		std::cout << e.what();
	}

	return fileText;
}

std::vector<std::string>* TextParser::parseTextFileByLine(std::string location)
{
	std::vector<std::string>* newStringVector = new std::vector<std::string>();

	std::string line;
	std::ifstream myFile(location);

	try
	{
		if (myFile.is_open())
		{
			while (getline(myFile, line))
			{
				newStringVector->push_back(line);
			}
			myFile.close();
			return newStringVector;
		}
		else
		{
			throw std::exception("Unable to open file\n");
		}
	}
	catch (std::exception e)
	{
		//Logger::log_error
		//Couldn't read the file
		delete newStringVector;
		return nullptr;
	}
}

void TextParser::print(std::string stringToPrint)
{
	std::cout << stringToPrint << "\n";
}

std::vector<std::string>* TextParser::splitString(std::string stringToSplit, char delimiter)
{
	std::vector<std::string>* newStringVector = new std::vector<std::string>();

	std::stringstream ss(stringToSplit);
	std::string temp = "";
	char i;

	while (ss >> i)
	{
		temp += i;
		char nextChar = ss.peek();

		if (nextChar == delimiter)
		{
			ss.ignore();

			newStringVector->push_back(temp);

			temp = "";
		}
	}

	//Got to the end of the text file. So whatever was in temp is the last string
	newStringVector->push_back(temp);

	return newStringVector;
}
