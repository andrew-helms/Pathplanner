#include "TileMap.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

TileMap::TileMap()
{
	//Does nothing, nothing is needed to be initialized it's done in initializemap so that we can use that
	//function to reset the map entirely if we later want to add that feature.
}

void TileMap::InitializeMap()
{
	PathTextureYellow.loadFromFile("images/WhiteYellow_Square_Black_Border_16_16.png");
	PathTextureGreen.loadFromFile("images/WhiteGreen_Square_Black_Border_16_16.png");
	PathTextureRed.loadFromFile("images/WhiteRed_Square_Black_Border_16_16.png");
	ObstacleTexture.loadFromFile("images/Rock_Obstacle_16_16.png");
	StateSpaceTexture.loadFromFile("images/LightBlue_Square_Black_Border_16_16.png");
	TileTexture.loadFromFile("images/Blue_Square_Black_Border_16_16.png");
	//Creating the tilemap tiles [it's a 47 by 57 of size 16 by 16 squares]. The states space map defaults as all obstacles.
	StateVector.clear();
	StateTraits.clear();
	TileVector.clear();
	TileTraits.clear();
	int Parser = 0;
	while (Parser < 47)
	{
		std::vector<sf::RectangleShape> StateSpaceRow;
		std::vector<std::vector<bool > > StateSpaceTraitsRow;
		std::vector<sf::RectangleShape> CurrentRow;
		std::vector<std::vector<bool > > TraitsRow;
		int InnerParser = 0;
		while (InnerParser < 57)
		{
			sf::RectangleShape Tile(sf::Vector2f(32.0, 32.0));
			sf::Vector2f Position(16.0f * (Parser)+112, 16.0f * (InnerParser)-8.0f);
			Tile.setPosition(Position);
			Tile.setTexture(&TileTexture);
			CurrentRow.push_back(Tile);
			std::vector<bool> Traits;
			TraitsRow.push_back(Traits);

			sf::RectangleShape Obstacle(sf::Vector2f(32.0, 32.0));
			Obstacle.setTexture(&ObstacleTexture);
			Obstacle.setPosition(Position);
			StateSpaceRow.push_back(Obstacle);
			std::vector<bool> StateSpaceTraitsObst;
			StateSpaceTraitsObst.push_back(true);
			StateSpaceTraitsRow.push_back(StateSpaceTraitsObst);

			InnerParser++;
		}
		TileVector.push_back(CurrentRow);
		TileTraits.push_back(TraitsRow);

		StateVector.push_back(StateSpaceRow);
		StateTraits.push_back(StateSpaceTraitsRow);
		Parser++;
	}
}

void TileMap::SetMapTexture(std::string InputImage)
{
	//Note: Currently doesn't actually change the map, to get it to do that need to parse through the map
	//and set the texture of each non-obstacle element to the new texture.
	TileTexture.loadFromFile(InputImage);
}

void TileMap::SetTrait(int xPos, int yPos, int TraitIndex, bool value)
{
	while (TileTraits[xPos][yPos].size() <= TraitIndex)
	{
		TileTraits[xPos][yPos].push_back(false); //Adding elements if they aren't added, rather than doing this above
		//statically doing this here lets me add more traits without having to worry about it.
	}
	TileTraits[xPos][yPos][TraitIndex] = value;
}

void TileMap::ResetTile(int xPos, int yPos)
{
	sf::RectangleShape Tile(sf::Vector2f(32.0, 32.0));
	sf::Vector2f Position(16.0f * (xPos)+113, 16.0f * (yPos)-8.0f);
	Tile.setPosition(Position);
	Tile.setTexture(&TileTexture);
	TileVector[xPos][yPos] = Tile;
	TileTraits[xPos][yPos].clear();
}

void TileMap::SaveMap(std::vector<DataNode> Agents)
{
	//All maps are going to be named "Map + i" where i is an integer 1-[infinity], so we can find out how many maps there are
	//by opening each and testing if we opened anything.
	int NumMaps = 1;
	std::string FileName = (std::string)"SavedMaps/" + "Map" + std::to_string(NumMaps) + (std::string)".txt";
	std::fstream MapFileIn;
	MapFileIn.open(FileName, std::fstream::in);
	//Needs to be ifstream or you'll create a file when you open it.
	while (MapFileIn.is_open())
	{
		//Whenver it works, go to the next one until it fails.
		MapFileIn.close();
		NumMaps++;
		FileName = (std::string)"SavedMaps/" + "Map" + std::to_string(NumMaps) + (std::string)".txt";
		MapFileIn.open(FileName, std::fstream::in);
	}
	//Now our current NumMaps integer indicates an amount of maps we haven't saved yet (so how many maps we have plus one).

	FileName = (std::string)"SavedMaps/" + "Map" + std::to_string(NumMaps) + (std::string)".txt";
	std::fstream MapFileOut(FileName, std::fstream::out);
	//Given background tiles 1-Infinity, (we currently have one), we find out which we're using and then set that to
	//be a 1-Infinity in the file. [However far we want].
	//1 is for the blue square with black border.
	//If we run into obstacles, we put an o before the # to indicate we're at an obstacle. Again 1-infinity.
	//o1 is for the rock obstacle.
	//Values are separated by commas.
	int Parser = 0;
	while (Parser < TileVector.size())
	{
		int InnerParser = 0;
		while (InnerParser < TileVector[Parser].size())
		{
			//First check if we have an obstacle.
			bool NoObstacle = true;
			if (!(TileTraits[Parser][InnerParser].size() == 0))
			{
				if (TileTraits[Parser][InnerParser][0])
				{
					NoObstacle = false;
				}
			}
			if (NoObstacle)
			{
				//Now we check if there's an obstacle in the state space or not.
				if (StateTraits[Parser][InnerParser][0])
				{
					MapFileOut << "1,";
				}
				//If there's no obstacle in either, we'll put a two.
				else
				{
					MapFileOut << "2,";
				}
			}
			else
			{
				//If there is an obnstacle here, there is one in the state space and out.
				//If there is an obstacle put "o1," as we only have the rock obstacle atm.
				MapFileOut << "o1,";
			}
			InnerParser++;
		}
		Parser++;
	}
	//Now that we've saved the map we'll save the character's data. We'll do each sequence in the order of the character
	//(Yellow, Green, Red).
	Parser = 0;
	while (Parser < Agents.size())
	{
		//First whether they're drawn:
		if (Agents[Parser].Drawn)
		{
			MapFileOut << "D,";
		}
		else
		{
			MapFileOut << "N,";
		}
		//Then their location:
		MapFileOut << std::to_string(Agents[Parser].CurrentLocation[0]) + ',';
		MapFileOut << std::to_string(Agents[Parser].CurrentLocation[1]) + ',';
		//Then their goal location:
		MapFileOut << std::to_string(Agents[Parser].GoalLocation[0]) + ',';
		MapFileOut << std::to_string(Agents[Parser].GoalLocation[1]) + ',';
		std::vector<std::vector<int> > CurActions = Agents[Parser].Actions;
		//Their number of actions
		MapFileOut << std::to_string(CurActions.size()) + ',';
		//Their actions
		int InnerParser = 0;
		while (InnerParser < CurActions.size())
		{
			MapFileOut << std::to_string(CurActions[InnerParser][0]) + ',';
			MapFileOut << std::to_string(CurActions[InnerParser][1]) + ',';
			InnerParser++;
		}
		Parser++;
	}
}

std::vector<DataNode> TileMap::LoadMap(int MapIndex)
{
	std::vector<DataNode> ReturnData;
	InitializeMap();
	//First we make sure our MapIndex exists (it should, we only call this internally, but if it doesn't we can return
	//false to indicate that we're at the end so hitting the forward arrow loops back.
	std::string FileName = (std::string)"SavedMaps/" + "Map" + std::to_string(MapIndex) + (std::string)".txt";
	std::ifstream MapFileIn;
	//Our map is 47 to 57, we start at 0 and every time we pass it we increase by 1 along the 47,
	//once we reach 47 we reset to 0 and increase 57 by one, this is needed to get the right position
	//of the tiles.
	int xLoc = 0;
	int yLoc = 0;
	MapFileIn.open(FileName);
	if (MapFileIn.is_open())
	{
		//If it opened then it exists and we can continue loading the map.
		//Used b as we'll use a for agent eventually if we have agents in the path.
		char c = 'b';
		while (MapFileIn.good() && (xLoc < 47))
		{
			std::string CurrentInput = "";
			while ((MapFileIn >> std::noskipws >> c)&&(!(c == ',')))
			{
				CurrentInput = CurrentInput + c;
			}
			//We don't need to worry about yLoc getting to 57 as it should stop at 46, 56 for any map file.
			//Once we're here we have reached our comma as c == ',', so we have our input. First we'll check if
			//we're on an obstacle.
			//Refer to initializemap for these conversions, just setting pixels locations.
			float xPosition = (xLoc * 16) + 112.0;
			float yPosition = (yLoc * 16) - 8;
			if (CurrentInput[0] == 'o')
			{
				//In this case we add our obstacles, as we only have one obstacle we know the next term is just one.
				sf::RectangleShape Obstacle(sf::Vector2f(32.0, 32.0));
				//Note: Optimization error here it's better to load it once and not have to load it again unless
				//you're having to change obstacles, for sufficiently large maps arguably ideal to just have a new
				//texture for every obstacle if you've way more map tiles than obstacles.
				ObstacleTexture.loadFromFile("images/Rock_Obstacle_16_16.png");
				Obstacle.setTexture(&ObstacleTexture);
				Obstacle.setPosition(sf::Vector2f(xPosition, yPosition));
				TileVector[xLoc][yLoc] = Obstacle;
				//Setting trait to there being an obstacle on the map, used SetTrait since that increments
				//the TileTraits[xLoc][yLoc] vector until it can fit the new addition, setting previous
				//ones to false.
				SetTrait(xLoc, yLoc, 0, true);
				
			}
			else if (CurrentInput[0] == '1')
			{
				//Here we have obstacle in state space but not in the normal realm.
				//If there wasn't an o there wasn't an obstacle there so there's an ordinary tile.
				sf::RectangleShape Tile(sf::Vector2f(32.0, 32.0));
				//No need to load this one it's loaded from setMapTexture.
				Tile.setTexture(&TileTexture);
				Tile.setPosition(sf::Vector2f(xPosition, yPosition));
				TileVector[xLoc][yLoc] = Tile;
			}
			else
			{
				//No obstacle in either space.
				sf::RectangleShape Tile(sf::Vector2f(32.0, 32.0));
				sf::RectangleShape StateTile(sf::Vector2f(32.0, 32.0));
				Tile.setTexture(&TileTexture);
				StateTile.setTexture(&StateSpaceTexture);
				Tile.setPosition(sf::Vector2f(xPosition, yPosition));
				StateTile.setPosition(sf::Vector2f(xPosition, yPosition));
				TileVector[xLoc][yLoc] = Tile;
				StateVector[xLoc][yLoc] = StateTile;
				StateTraits[xLoc][yLoc][0] = false;
			}
			//As we've finished an input we increment xLoc and check if it reached 47.
			yLoc++;
			if (yLoc >= 57)
			{
				xLoc++;
				yLoc = 0;
			}
		}
		//Now that we're here xLoc is at 47, meaning we just finished the map, time to read the three agents.
		int Parser = 0;
		int Tester = 0;
		while (Parser < 3)
		{
			DataNode CurrentAgent;
			//First we get the next item that isn't a comma.
			std::string CurrentInput = "";
			while ((c == ',') || (Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			//If it's a D then we draw the agent.
			if (CurrentInput[0] == 'D')
			{
				CurrentAgent.Drawn = true;
			}
			else
			{
				CurrentAgent.Drawn = false;
			}
			while ((c == ',') || (Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			CurrentInput = "";
			//Now we get the x coordinate of its location.
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			int xLoc = std::stoi(CurrentInput);
			CurrentInput = "";
			while ((c == ',')||(Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			int yLoc = std::stoi(CurrentInput);
			CurrentInput = "";

			CurrentAgent.CurrentLocation.push_back(xLoc);
			CurrentAgent.CurrentLocation.push_back(yLoc);
			while ((c == ',') || (Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			//Now its goal location
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			int xGoalLoc = std::stoi(CurrentInput);
			CurrentInput = "";

			while ((c == ',') || (Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			int yGoalLoc = std::stoi(CurrentInput);
			CurrentInput = "";

			CurrentAgent.GoalLocation.push_back(xGoalLoc);
			CurrentAgent.GoalLocation.push_back(yGoalLoc);

			while ((c == ',') || (Tester > 10))
			{
				MapFileIn >> c;
				Tester++;
			}
			Tester = 0;
			//Now we get the number of actions.
			while (!(c == ','))
			{
				CurrentInput = CurrentInput + c;
				MapFileIn >> std::noskipws >> c;
			}
			int ActionCount = std::stoi(CurrentInput);
			int InnerParser = 0;
			CurrentInput = "";
			while (InnerParser < ActionCount)
			{
				while ((c == ',') || (Tester > 10))
				{
					MapFileIn >> c;
					Tester++;
				}
				Tester = 0;
				//For each action we get the action's x coord
				while (!(c == ','))
				{
					CurrentInput = CurrentInput + c;
					MapFileIn >> std::noskipws >> c;
				}
				int xAction = std::stoi(CurrentInput);
				CurrentInput = "";

				while ((c == ',') || (Tester > 10))
				{
					MapFileIn >> c;
					Tester++;
				}
				Tester = 0;
				while (!(c == ','))
				{
					CurrentInput = CurrentInput + c;
					MapFileIn >> std::noskipws >> c;
				}
				int yAction = std::stoi(CurrentInput);
				std::vector<int> CurrentAction;
				CurrentAction.push_back(xAction); CurrentAction.push_back(yAction);
				CurrentAgent.Actions.push_back(CurrentAction);
				CurrentInput = "";

				InnerParser++;
			}
			ReturnData.push_back(CurrentAgent);
			Parser++;
		}
		return ReturnData;
	}
	else
	{
		return ReturnData;
	}
}

bool TileMap::IsObstacle(int xPos, int yPos)
{
	//Whether it's an obstacle is the first (and currently only) defined trait.
	if (TileTraits[xPos][yPos].size() == 0)
	{
		return false;
	}
	else
	{
		return TileTraits[xPos][yPos][0];
	}
}
