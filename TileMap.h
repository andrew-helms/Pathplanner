#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

struct DataNode
{
	bool Drawn;
	std::vector<int> CurrentLocation;
	std::vector<int> GoalLocation;
	std::vector<std::vector<int> > Actions;
};

class TileMap
{
	public:

	sf::Texture TileTexture; //Texture for the tile map.
	sf::Texture ObstacleTexture; //Texture for the obstacles.
	sf::Texture StateSpaceTexture;
	std::vector<std::vector<sf::RectangleShape> > TileVector; //Vector that stores all of the tiles.
	std::vector<std::vector<std::vector<bool> > > TileTraits; //Notes the traits of a tile (an obstacle? etc.)
	//The below is for the statespace, it defaults to having only obstacles until it is whited out manually then through loading.
	std::vector<std::vector<sf::RectangleShape> > StateVector;
	std::vector<std::vector<std::vector<bool> > > StateTraits;
	std::vector<std::vector<std::vector<int> > > AutoMovers;
	//Current traits:
	//0 == whether it's in obstacle or not.

	TileMap();
	void InitializeMap();
	void SetMapTexture(std::string InputImage);
	void SetTrait(int xPos, int yPos, int TraitIndex, bool value);
	void ResetTile(int xPos, int yPos);
	void SaveMap(std::vector<DataNode> Agents);
	std::vector<DataNode> LoadMap(int MapIndex); //Boolean to return false if the map didn't exist, otherwise true.
	bool IsObstacle(int xPos, int yPos);
};

