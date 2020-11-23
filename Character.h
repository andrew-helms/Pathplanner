#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include "TileMap.h"

class Character
{
	public:
	//The character class will store its own Tile and texture.
	sf::RectangleShape CharacterTile = sf::RectangleShape(sf::Vector2f(32.0f, 32.0f));
	sf::Texture CharacterTexture;
	//Boolean to check if we're currently drawing this character.
	bool DoDraw = false;
	//It will store its actions as a vector of integers (the inner vectors only pairs of ints) as well as its current location.
	std::vector<sf::Vector2f> Actions;
	sf::Vector2f CurrentLocation;
	std::vector<int> GoalLocation;
	
	//The input for creating the character is its texture image.
	Character(std::string ImageLocation);
	void SetLocation(sf::Vector2f NewLocation);
	void Move(int xPos, int yPos, TileMap& Map);
	std::vector<std::vector<int> > GetActions(); //GetActions method returns the Actions we have but converted from their vector2fs to the integers that correlate to
	//the actual tilemap so they work well with the move function (so intuitively you see the tilemap as (0,0) being the top left of the grid and actions
	//being integers that move you aong the grid by the expected squares.
	void AddAction(int xChange, int yChange); //Adds an action using integers.
	std::vector<int> GetLocation();
	void SetLocationInt(int xLoc, int yLoc);
	
};

