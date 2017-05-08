/*Created: Feb 17, 2017
	Author : Matthew Chiborak

	Description :
		 Class that units can pickup and bring back to base to increase their resources
			 */
#ifndef __PICKUP_H
#define __PICKUP_H

#include <SFML\Graphics.hpp>

class pickup
{
private:
	int value;
	sf::Sprite pickupSprite;
public:
	pickup(sf::Texture* pickupTexture, const float size, const int value);
	~pickup();
	sf::Sprite* getSprite();
	void addToValue(int addition);
	int getValue();
};

#endif