#include "Character.h"
#include <iostream>

Character::Character(std::string InputLocation)
{
	CharacterTexture.loadFromFile(InputLocation);
	CharacterTile.setTexture(&CharacterTexture);
}

void Character::SetLocation(sf::Vector2f NewLocation)
{
	CharacterTile.setPosition(NewLocation);
	CurrentLocation = NewLocation;
}

void Character::Move(int xLoc, int yLoc, TileMap& Map)
{
	std::vector<int> CurLocation = GetLocation();
	int xNewLoc = xLoc + CurLocation[0];
	int yNewLoc = yLoc + CurLocation[1];
	//First we check if we're moving into an obstacle.
	bool NoObstacle = true;
	if (!(Map.TileTraits[xNewLoc][yNewLoc].size() == 0))
	{
		if (Map.TileTraits[xNewLoc][yNewLoc][0])
		{
			NoObstacle = false;
		}
	}
	if (NoObstacle)
	{
		//To move there we convert our location into pixel location.
		float xPixelLoc = ((xNewLoc + 7) * 16) + 1;
		float yPixelLoc = yNewLoc * 16 - 8;
		SetLocation(sf::Vector2f(xPixelLoc, yPixelLoc));
	}
}

std::vector<std::vector<int> > Character::GetActions()
{
	std::vector<std::vector<int> > NewActions;
	int Parser = 0;
	while (Parser < Actions.size())
	{
		std::vector<int> CurrentAction;
		//We convert from the pixel-change of the vectors to the grid-change.
		int xVal = (Actions[Parser].x + 1) / 16;
		int yVal = (Actions[Parser].y - 8) / 16;
		CurrentAction.push_back(xVal);
		CurrentAction.push_back(yVal);
		NewActions.push_back(CurrentAction);
		Parser++;
	}
	return NewActions;
}

std::vector<int> Character::GetLocation()
{
	std::vector<int> CurrentLocationInt;
	int xLoc = (CurrentLocation.x - 113) / 16;
	int yLoc = (CurrentLocation.y + 8) / 16;
	CurrentLocationInt.push_back(xLoc);
	CurrentLocationInt.push_back(yLoc);
	return CurrentLocationInt;
}

void Character::SetLocationInt(int xLoc, int yLoc)
{
	float xPixLoc = (xLoc * 16) + 113;
	float yPixLoc = (yLoc * 16) - 8;
	SetLocation(sf::Vector2f(xPixLoc, yPixLoc));
}

void Character::AddAction(int xChange, int yChange)
{
	float xChangePix = (xChange * 16) - 1;
	float yChangePix = (yChange * 16) + 8;
	Actions.push_back(sf::Vector2f(xChangePix, yChangePix));
}