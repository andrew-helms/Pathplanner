#include <SFML/Graphics.hpp> //Used to render the windows.
#include <vector> //Used for creating the map.
#include <iostream> //Used for debugging.
#include <math.h> //Used for the floor function.
#include <chrono> //Chrono and thread are used to delay the program for adding toggles (so they don't flicker in the seconds you're holding the button)
#include <thread>

#include "Character.h" //Used for characters.
#include "TileMap.h" //The tile map.
#include "PathFinder.h" //AStar algorithm and related includes
using namespace std::chrono_literals; //Namespace to not write it when using this_thread and sleep_for

//Pathing render-system: takes in a series of actions (std::vector<std::vector<int> >, each action being size 2) and moves the player along the
//path one by one, pausing so we can visually see it, designed to go one along the path each time it's called so that when the
//while(window.isOpen()) calls it a ton of times it only moves once per call so we visually see each move.
//**Note: Path is currently designed only to be called once per run of the program.**
int PathCounter = -1;
std::vector<bool> Pathing(1, true); //Made a vector so we can path multiple times, set initial value to true and after that every
//time we run Path it will (once the previous path is done) add a new element to the vector and set that to true and the current
//one to false so that we can begin the next path. Added an integer to the Path function input to choose which path we're currently
//running (so PathCount = 0 is for the first path, PathCount = 1 the second, etc.))
//are added to it.
//We will increment PathingCount each time we run a new path.
int PathingCount = 0;
//These are the pathes for each agent, ActionsPerAgent[0] is the path for Agents[0]
std::vector<std::vector<std::vector<int > > > ActionsPerAgent;
std::vector<Character*> Agents;
std::vector<Character> AgentPreserver;

//Map for character PoV
TileMap CharacterPOV;
bool SeeCharacterPOV = false;

//Booleans for changing maps during loading them (makes it more responsive)
bool PressedLeft = false;
bool PressedRight = false;
bool ChosenMap = false;

void PathFunc(TileMap& Map, std::vector<bool>& Pathing, int PathCount)
{
	//First we'll check if we're currently pathing (this is to avoid checking Pathing[PathCount] when the vector doesn't have an
	//element at that point.
	bool CurrentlyPathing = false;
	if (Pathing.size() > PathCount)
	{
		CurrentlyPathing = Pathing[PathCount];
	}
	if (PathCounter == -1)
	{
		PathCounter++; //This is done so that we can see the map before any pathing is done.
	}
	else if (CurrentlyPathing)
	{
		//We'll parse through each of the pathactions, with PathActions[0] corresponding to the inputted Agents[0].
		int CurrentAgent = 0;
		while (CurrentAgent < ActionsPerAgent.size())
		{
			if (Agents[CurrentAgent]->GetLocation()[0] != Agents[CurrentAgent]->GoalLocation[0] || Agents[CurrentAgent]->GetLocation()[1] != Agents[CurrentAgent]->GoalLocation[1])
			{
				int Parser = 0;
				std::vector<int> CurrentLocation = Agents[CurrentAgent]->GetLocation();
				while (Parser < ActionsPerAgent[CurrentAgent].size())
				{
					if (SeeCharacterPOV)
					{
						if (CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureYellow)
						{
							if (CharacterPOV.TileTraits[CurrentLocation[0]][CurrentLocation[1]].size() == 0)
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
							}
							else
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.ObstacleTexture);
							}
						}
						if (CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureGreen)
						{
							if (CharacterPOV.TileTraits[CurrentLocation[0]][CurrentLocation[1]].size() == 0)
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
							}
							else
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.ObstacleTexture);
							}
						}
						if (CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureRed)
						{
							if (CharacterPOV.TileTraits[CurrentLocation[0]][CurrentLocation[1]].size() == 0)
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
							}
							else
							{
								CharacterPOV.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.ObstacleTexture);
							}
						}
					}
					else
					{
						if (Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureYellow)
						{
							Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
						}
						if (Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureGreen)
						{
							Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
						}
						if (Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].getTexture() == &Map.PathTextureRed)
						{
							Map.TileVector[CurrentLocation[0]][CurrentLocation[1]].setTexture(&Map.TileTexture);
						}
					}
					std::vector<int> NewLocation;
					NewLocation.push_back(CurrentLocation[0] + ActionsPerAgent[CurrentAgent][Parser][0]);
					NewLocation.push_back(CurrentLocation[1] + ActionsPerAgent[CurrentAgent][Parser][1]);
					CurrentLocation = NewLocation;
					Parser++;
				}

				std::vector<std::vector<std::vector<bool>>> maskedObstacles = Map.TileTraits;
				std::vector<int> CurrentAction = ActionsPerAgent[CurrentAgent][0];
				Agents[CurrentAgent]->Move(CurrentAction[0], CurrentAction[1], Map);
				for (int col = 0; col < maskedObstacles.size(); col++)
					for (int row = 0; row < maskedObstacles[0].size(); row++)
						if (maskedObstacles[col][row].size() != 0 && abs(col - Agents[CurrentAgent]->GetLocation()[0]) <= Agents[CurrentAgent]->radius && abs(row - Agents[CurrentAgent]->GetLocation()[1]) <= Agents[CurrentAgent]->radius)
						{
							if (maskedObstacles[col][row][0])
							{
								if (Agents[CurrentAgent]->KnownMap.TileTraits[col][row].size() == 0)
								{
									Agents[CurrentAgent]->KnownMap.TileTraits[col][row].push_back(true);
									CharacterPOV.TileTraits[col][row].push_back(true);
									CharacterPOV.TileVector[col][row].setTexture(&CharacterPOV.ObstacleTexture);
								}
								else
								{
									Agents[CurrentAgent]->KnownMap.TileTraits[col][row][0] = true;
									CharacterPOV.TileTraits[col][row].push_back(true);
									CharacterPOV.TileVector[col][row].setTexture(&CharacterPOV.ObstacleTexture);
								}
							}
							//If there isn't an obstacle in the location, net CharacterPOV to color the area to show the character's radius.
						}
				
				Agents[CurrentAgent]->path = Agents[CurrentAgent]->pfa->Update(Agents[CurrentAgent]->GetActions(), Agents[CurrentAgent]->KnownMap.TileTraits, Agents[CurrentAgent]->GetLocation(), Agents[CurrentAgent]->GoalLocation);
				ActionsPerAgent[CurrentAgent] = Agents[CurrentAgent]->path.path;
				//Coloring the path for visual purposes:
				CurrentLocation = Agents[CurrentAgent]->GetLocation();
				Parser = 0;
				while (Parser < ActionsPerAgent[CurrentAgent].size())
				{
					std::vector<int> NewLocation;
					NewLocation.push_back(CurrentLocation[0] + ActionsPerAgent[CurrentAgent][Parser][0]);
					NewLocation.push_back(CurrentLocation[1] + ActionsPerAgent[CurrentAgent][Parser][1]);
					if (SeeCharacterPOV)
					{
						if (CharacterPOV.TileTraits[NewLocation[0]][NewLocation[1]].size() == 0)
						{
							if (Agents[CurrentAgent]->AgentType == 1)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureYellow);
							}
							else if (Agents[CurrentAgent]->AgentType == 2)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureGreen);
							}
							else if (Agents[CurrentAgent]->AgentType == 3)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureRed);
							}
						}
						else if (CharacterPOV.TileTraits[NewLocation[0]][NewLocation[1]][0] == false)
						{
							if (Agents[CurrentAgent]->AgentType == 1)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureYellow);
							}
							else if (Agents[CurrentAgent]->AgentType == 2)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureGreen);
							}
							else if (Agents[CurrentAgent]->AgentType == 3)
							{
								CharacterPOV.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureRed);
							}
							//Do nothing otherwise it's an obstacle.
						}
					}
					if (Map.TileTraits[NewLocation[0]][NewLocation[1]].size() == 0)
					{
						if(!(SeeCharacterPOV))
						{
							if (Agents[CurrentAgent]->AgentType == 1)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureYellow);
							}
							else if (Agents[CurrentAgent]->AgentType == 2)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureGreen);
							}
							else if (Agents[CurrentAgent]->AgentType == 3)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureRed);
							}
						}
					}
					else if (Map.TileTraits[NewLocation[0]][NewLocation[1]][0] == false)
					{
						if(!(SeeCharacterPOV))
						{
							if (Agents[CurrentAgent]->AgentType == 1)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureYellow);
							}
							else if (Agents[CurrentAgent]->AgentType == 2)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureGreen);
							}
							else if (Agents[CurrentAgent]->AgentType == 3)
							{
								Map.TileVector[NewLocation[0]][NewLocation[1]].setTexture(&Map.PathTextureRed);
							}
						}
						//Do nothing otherwise it's an obstacle.
					}
					CurrentLocation = NewLocation;
					Parser++;
				}
			}
			CurrentAgent++;
		}
		PathCounter++;
		bool FinishedPathing = true;
		CurrentAgent = 0;
		while (CurrentAgent < ActionsPerAgent.size())
		{
			if(Agents[CurrentAgent]->GetLocation()[0] != Agents[CurrentAgent]->GoalLocation[0] || Agents[CurrentAgent]->GetLocation()[1] != Agents[CurrentAgent]->GoalLocation[1])
			{
				FinishedPathing = false;
			}
			CurrentAgent++;
		}
		if (FinishedPathing)
		{
			//Reset CharacterPOV when finished pathing.
			CharacterPOV.InitializeMap();
			for (int i = 0; i < ActionsPerAgent.size(); i++)
			{
				if (i == 0)
				{
					std::cout << "Agent Yellow's path data: Nodes Expanded = " << Agents[i]->path.nodesExpanded << ", Execution Time = " << Agents[i]->path.exeTime << std::endl;
				}
				else if (i == 1)
				{
					std::cout << "Agent Green's path data: Nodes Expanded = " << Agents[i]->path.nodesExpanded << ", Execution Time = " << Agents[i]->path.exeTime << std::endl;
				}
				else if (i == 2)
				{
					std::cout << "Agent Red's path data: Nodes Expanded = " << Agents[i]->path.nodesExpanded << ", Execution Time = " << Agents[i]->path.exeTime << std::endl;
				}
			}

			Pathing[PathCount] = false;
			if (Pathing.size() == PathCount + 1)
			{
				Pathing.push_back(true);
			}
			else
			{
				Pathing[PathCount + 1] = true;
			}
			PathCounter = -1;
			ActionsPerAgent.clear();
			Agents.clear();
		}
		std::this_thread::sleep_for(0.1s); //Sleeping so we visually see the change.
	}
};

int main()
{
	//Initializing characterPOV
	CharacterPOV.InitializeMap();

	//Creating the Tilemap.
	TileMap Map;
	//Loading the texture for the map tiles.
	Map.SetMapTexture("images/Blue_Square_Black_Border_16_16.png");
	//Initialize the map.
	Map.InitializeMap();

	//Creating a tilemap for when loading maps.
	TileMap LoadingMap;
	LoadingMap.SetMapTexture("images/Blue_Square_Black_Border_16_16.png");
	LoadingMap.InitializeMap();

	//Creating an array of character objects, this will allow us to create any from 1-3 characters so we can show
	//the implementation of each pathing method on the map at once. We only draw them depending on
	//whether or not we've created said character.
	Character* TargetAgent;
	Character AgentYellow("images/Yellow_Square_Black_Border_16_16.png");
	AgentYellow.AgentType = 1;
	Character AgentGreen("images/Green_Square_Black_Border_16_16.png");
	AgentGreen.AgentType = 2;
	Character AgentRed("images/Red_Square_Black_Border_16_16.png");
	AgentRed.AgentType = 3;

	//Creating the texture for tiles that indicate the player can move to the location.
	sf::Texture ActionTileTexture;
	ActionTileTexture.loadFromFile("images/Orange_Square_16_16.png");

	//Creating the texture for obstacle tiles
	sf::Texture ObstacleTileTexture;
	ObstacleTileTexture.loadFromFile("images/Rock_Obstacle_16_16.png");
	//Create a vector to store all of the obstacles' positions.
	std::vector<sf::Vector2f> ObstaclePositions;


	//Creating the User Interface.
	//Creating the Create_Button texture.
	sf::Texture CreateButton;
	CreateButton.loadFromFile("images/Create_Button_64_64.png");
	sf::RectangleShape CreateButtonTile(sf::Vector2f(64.0, 64.0));
	CreateButtonTile.setTexture(&CreateButton);
	//Default position is (0,0) where we want it no need to set position.

	//Booleans to preserve button presses (so if the create button is hit the text and effects that appear preserve until the user interacts with it).
	bool CreateButtonPressed = false;

	//Creating the Set_target texture.
	sf::Texture SetTarget;
	SetTarget.loadFromFile("images/Set_Target_64_64.png");
	sf::RectangleShape SetTargetTile(sf::Vector2f(64.0, 64.0));
	SetTargetTile.setTexture(&SetTarget);
	SetTargetTile.setPosition(sf::Vector2f(944.0, 0.0));
	sf::RectangleShape CurrentTargetTile(sf::Vector2f(32.0, 32.0));
	CurrentTargetTile.setPosition(944.0, 128.0);
	sf::Texture YellowTexture;
	YellowTexture.loadFromFile("images/Yellow_Square_Black_Border_16_16.png");
	sf::RectangleShape AgentYellowTile(sf::Vector2f(32.0, 32.0));
	AgentYellowTile.setPosition(976.0, 128.0);
	AgentYellowTile.setTexture(&YellowTexture);
	CurrentTargetTile.setTexture(&YellowTexture);
	sf::Texture GreenTexture;
	GreenTexture.loadFromFile("images/Green_Square_Black_Border_16_16.png");
	sf::RectangleShape AgentGreenTile(sf::Vector2f(32.0, 32.0));
	AgentGreenTile.setPosition(976.0, 152.0);
	AgentGreenTile.setTexture(&GreenTexture);
	sf::Texture RedTexture;
	RedTexture.loadFromFile("images/Red_Square_Black_Border_16_16.png");
	sf::RectangleShape AgentRedTile(sf::Vector2f(32.0, 32.0));
	AgentRedTile.setPosition(976.0, 176.0);
	AgentRedTile.setTexture(&RedTexture);

	//Creating the path button
	sf::Texture Path;
	Path.loadFromFile("images/Path_64_64.png");
	sf::RectangleShape PathTile(sf::Vector2f(64.0, 64.0));
	PathTile.setTexture(&Path);
	//Setting the Path's button position to be on the right side, below the selection of characters.
	PathTile.setPosition(944.0, 224.0);

	//Creating the path all button
	sf::Texture PathAll;
	PathAll.loadFromFile("images/Path_All_64_64.png");
	sf::RectangleShape PathAllTile(sf::Vector2f(64.0, 64.0));
	PathAllTile.setTexture(&PathAll);
	PathAllTile.setPosition(944.0, 288.0);

	//Creating the state space button
	sf::Texture StateSpace;
	StateSpace.loadFromFile("images/State_Space_64_64.png");
	sf::RectangleShape StateSpaceTile(sf::Vector2f(64.0, 64.0));
	StateSpaceTile.setTexture(&StateSpace);
	StateSpaceTile.setPosition(944.0, 352.0);

	bool StateSpaceMode = false;

	/*Creating Auto Pather button*/
	sf::Texture AutoPatherTexture;
	AutoPatherTexture.loadFromFile("images/Add_Auto_Pather_64_64.png");
	sf::RectangleShape AutoPatherTile(sf::Vector2f(64.0, 64.0));
	AutoPatherTile.setTexture(&AutoPatherTexture);
	AutoPatherTile.setPosition(944.0, 416.0);
	/*Auto pather code section here over*/

	//Creating the Char POV button
	sf::Texture CharPOVTexture;
	CharPOVTexture.loadFromFile("images/Char_POV_64_64.png");
	sf::RectangleShape CharPOVTile(sf::Vector2f(64.0, 64.0));
	CharPOVTile.setTexture(&CharPOVTexture);
	CharPOVTile.setPosition(944.0, 480.0);


	//Creating the Set_character texture.
	sf::Texture SetCharacter;
	SetCharacter.loadFromFile("images/Set_Char_64_64.png");
	sf::RectangleShape SetCharacterTile(sf::Vector2f(64.0, 64.0));
	SetCharacterTile.setTexture(&SetCharacter);
	//Setting Set Character button's position to be below create character.
	SetCharacterTile.setPosition(sf::Vector2f(0.0, 64.0));

	//boolean for SetCharacter.
	bool SetCharacterPressed = false;

	//Creating the ToggleAddMoves button
	sf::Texture ToggleAddMoves;
	ToggleAddMoves.loadFromFile("images/Toggle_Add_Moves_64_64.png");
	sf::RectangleShape ToggleMovesTile(sf::Vector2f(64.0, 64.0));
	ToggleMovesTile.setTexture(&ToggleAddMoves);
	//Setting ToggleAddMoves button's position to be below set character by 64 extra pixels.
	ToggleMovesTile.setPosition(sf::Vector2f(0.0, 192.0));

	//boolean for ToggleAddMoves
	bool AddingMoves = false;

	//Creating the ToggleObstacles button
	sf::Texture ToggleObstacles;
	ToggleObstacles.loadFromFile("images/Toggle_Obstacles_64_64.png");
	sf::RectangleShape ToggleObstaclesTile(sf::Vector2f(64.0, 64.0));
	ToggleObstaclesTile.setTexture(&ToggleObstacles);
	//setting ToggleObstacle button's position to be below toggle moves exactly.
	ToggleObstaclesTile.setPosition(sf::Vector2f(0.0, 256.0));

	//boolean for ToggleObstacles
	bool AddingObstacles = false;

	//Creating the Remove Toggle button
	sf::Texture RemoveToggle;
	RemoveToggle.loadFromFile("images/Remove_Toggle_64_64.png");
	sf::RectangleShape RemoveToggleTile(sf::Vector2f(64.0, 64.0));
	RemoveToggleTile.setTexture(&RemoveToggle);
	//Setting Remove obstacle's button position to be below toggle obstacles.
	RemoveToggleTile.setPosition(sf::Vector2f(0.0, 320.0));

	//boolean for Remove Toggle.
	bool RemoveToggled = false;

	//Creating the move character button
	sf::Texture MoveCharacter;
	MoveCharacter.loadFromFile("images/Move_Character_64_64.png");
	sf::RectangleShape MoveCharacterTile(sf::Vector2f(64.0, 64.0));
	MoveCharacterTile.setTexture(&MoveCharacter);
	//Setting Move character button's position to be below remove toggle's button.
	MoveCharacterTile.setPosition(sf::Vector2f(0.0, 384.0));

	//Boolean for moving character toggle.
	bool MovingCharacter = false;

	//Creating the Set Path button
	sf::Texture SetPath;
	SetPath.loadFromFile("images/Set_Path_64_64.png");
	sf::RectangleShape SetPathTile(sf::Vector2f(64.0, 64.0));
	SetPathTile.setTexture(&SetPath);
	//Setting the Set Path's button position to be below the move character toggle.
	SetPathTile.setPosition(sf::Vector2f(0.0, 448.0));

	//Creating a bool for set path
	bool SettingPath = false;

	//Creating the save map button
	sf::Texture SaveMap;
	SaveMap.loadFromFile("images/Save_Map_64_64.png");
	sf::RectangleShape SaveMapTile(sf::Vector2f(64.0, 64.0));
	SaveMapTile.setTexture(&SaveMap);
	//Setting the save map button's position to be 64 extra below the remove toggle's button.
	SaveMapTile.setPosition(sf::Vector2f(0.0, 572.0));

	//Creating a bool for saving a map so we can know whether to send the text.
	bool SavedMap = false;
	
	//Creating the load map button
	sf::Texture LoadMap;
	LoadMap.loadFromFile("images/Load_Map_64_64.png");
	sf::RectangleShape LoadMapTile(sf::Vector2f(64.0, 64.0));
	LoadMapTile.setTexture(&LoadMap);
	//Setting the load map button's position to be exactly below the save map button.
	LoadMapTile.setPosition(sf::Vector2f(0.0, 636.0));

	//Creating a bool for entering load map state (so to draw the maps inside of the area rather than the actual map).
	bool LoadingMapMode = false;
	//Integer for storing what map we're looking at.
	int CurrentMap = 1;

	//Creating the right arrow for loading map.
	sf::Texture RightArrow;
	RightArrow.loadFromFile("images/Right_Arrow_64_64.png");
	sf::RectangleShape RightArrowTile(sf::Vector2f(64.0, 64.0));
	RightArrowTile.setTexture(&RightArrow);
	//Setting the right arrow button's position to be where the create button's text (bottom area)
	RightArrowTile.setPosition(sf::Vector2f(600.0, 912.0));

	//Creating the left arrow for loading map.
	sf::Texture LeftArrow;
	LeftArrow.loadFromFile("images/Left_Arrow_64_64.png");
	sf::RectangleShape LeftArrowTile(sf::Vector2f(64.0, 64.0));
	LeftArrowTile.setTexture(&LeftArrow);
	//Setting the left arrow's position to be where the create button's text is (bottom area) as well.
	LeftArrowTile.setPosition(sf::Vector2f(344.0, 912.0));

	//Creating the Select map button for loading map.
	sf::Texture SelectMap;
	SelectMap.loadFromFile("images/Select_Map_64_64.png");
	sf::RectangleShape SelectMapTile(sf::Vector2f(64.0, 64.0));
	SelectMapTile.setTexture(&SelectMap);
	//Setting the select map button's position to be in the middle of the two arrows.
	SelectMapTile.setPosition(sf::Vector2f(472.0, 912.0));

	//Creating a cancel button to cancel all current actions (set all of the booleans to false)
	//Note: Doesn't set SavedMap as that sets itself to false.
	sf::Texture CancelButton;
	CancelButton.loadFromFile("images/Cancel_Button_64_64.png");
	sf::RectangleShape CancelButtonTile(sf::Vector2f(64.0, 64.0));
	CancelButtonTile.setTexture(&CancelButton);
	//Its position will be just above the text for adding moves.
	CancelButtonTile.setPosition(sf::Vector2f(0.0, 848.0));


	//Loading the font (appears at bottom of screen for UI for user) for creating character.
	sf::Font Font;
	sf::Text Text;
	Font.loadFromFile("Fonts/game_over.ttf");
	Text.setFont(Font);
	Text.setFillColor(sf::Color::White);

	//This is text's default position (bottom left corner just under the map of tiles), when moving it just note that's where it starts for reference.
	Text.setPosition(118.0f, 880.0f);
	Text.setCharacterSize(72);

	//Creating another text for inidcating toggles.
	sf::Text ToggleIndicatorText;
	ToggleIndicatorText.setFont(Font);
	ToggleIndicatorText.setFillColor(sf::Color::White);
	ToggleIndicatorText.setPosition(0.0f, 880.0f);
	ToggleIndicatorText.setCharacterSize(72);

	//Creating a text for loading map (noting which map is being displayed).
	sf::Text LoadingMapText;
	LoadingMapText.setFont(Font);
	LoadingMapText.setFillColor(sf::Color::White);
	LoadingMapText.setPosition(664.0, 880.0);
	LoadingMapText.setCharacterSize(72);

	/**********************************************************/
	/*             Pathfinding Code in main here              */
	//Sample code here is for you to understand how to use this.
	//If you want to load a map you created using the User Interface, run the LoadMap(int MapIndex) function where your map
	//is named EXACTLY "Map" + MapIndex in the SavedMaps folder. For example if you wanted Map6 you'd run
	// Map.LoadMap(6), I'm going to load map six so when you run it you'll see map six.
	std::vector<DataNode> Empty;
	TargetAgent = &AgentYellow;
	TargetAgent->DoDraw = true; //Set Player.DoDraw to true so it's seen without needing to hit create character.
	TargetAgent->SetLocationInt(21, 25);
	std::vector<int> CurrentLoc = TargetAgent->GetLocation();
	AgentGreen.SetLocationInt(40, 40);
	AgentGreen.AddAction(0, 1);
	AgentGreen.AddAction(1, 0);
	AgentGreen.AddAction(0, -1);
	AgentGreen.AddAction(-1, 0);
	std::vector<int> GLoc;
	GLoc.push_back(5);
	GLoc.push_back(5);
	AgentGreen.GoalLocation = GLoc;
	TargetAgent->AddAction(0, 1);
	TargetAgent->AddAction(1, 0);
	TargetAgent->AddAction(0, -1);
	TargetAgent->AddAction(-1, 0);
	GLoc[0] = 3;
	GLoc[1] = 2;
	TargetAgent->GoalLocation = GLoc;
	AgentRed.SetLocationInt(20, 2);
	AgentRed.AddAction(0, 1);
	AgentRed.AddAction(1, 0);
	AgentRed.AddAction(0, -1);
	AgentRed.AddAction(-1, 0);
	GLoc[0] = 40;
	GLoc[1] = 40;
	AgentRed.GoalLocation = GLoc;
	Empty = Map.LoadMap(7);
	if (Empty.size() != 0)
	{
		AgentYellow.SetLocationInt(Empty[0].CurrentLocation[0], Empty[0].CurrentLocation[1]);
		AgentYellow.GoalLocation[0] = Empty[0].GoalLocation[0];
		AgentYellow.GoalLocation[1] = Empty[0].GoalLocation[1];
		AgentYellow.DoDraw = Empty[0].Drawn;
		AgentYellow.Actions.clear();
		int ActionParser = 0;
		while (ActionParser < Empty[0].Actions.size())
		{
			AgentYellow.AddAction(Empty[0].Actions[ActionParser][0], Empty[0].Actions[ActionParser][1]);
			ActionParser++;
		}
		AgentGreen.SetLocationInt(Empty[1].CurrentLocation[0], Empty[1].CurrentLocation[1]);
		AgentGreen.GoalLocation[0] = Empty[1].GoalLocation[0];
		AgentGreen.GoalLocation[1] = Empty[1].GoalLocation[1];
		AgentGreen.DoDraw = Empty[1].Drawn;
		AgentGreen.Actions.clear();
		ActionParser = 0;
		while (ActionParser < Empty[1].Actions.size())
		{
			AgentGreen.AddAction(Empty[1].Actions[ActionParser][0], Empty[1].Actions[ActionParser][1]);
			ActionParser++;
		}
		AgentRed.SetLocationInt(Empty[2].CurrentLocation[0], Empty[2].CurrentLocation[1]);
		AgentRed.GoalLocation[0] = Empty[2].GoalLocation[0];
		AgentRed.GoalLocation[1] = Empty[2].GoalLocation[1];
		AgentRed.DoDraw = Empty[2].Drawn;
		AgentRed.Actions.clear();
		ActionParser = 0;
		while (ActionParser < Empty[2].Actions.size())
		{
			AgentRed.AddAction(Empty[2].Actions[ActionParser][0], Empty[2].Actions[ActionParser][1]);
			ActionParser++;
		}
	}

	//PathFinder* APathPlanner = new AStar(TargetAgent->GetActions(), Map.TileTraits);
	//PathFinder* pathPlanner = new LPA(TargetAgent->GetActions(), Map.TileTraits);
	//PathReturn* AStarPath = APathPlanner->Update(TargetAgent->GetActions(), Map.TileTraits, TargetAgent->GetLocation(), TestVect);
	//PathReturn* path = pathPlanner->Update(TargetAgent->GetActions(), Map.TileTraits, TargetAgent->GetLocation(), TestVect);
	//std::vector<std::vector<int> > Actions = AStarPath->path; //AStarPath works.
	//I currently haven't created a way to save characters, so actions need to be added manually using Player.AddAction(int x, int y).
	//For the path (note: see code up top for how it works: design an std::vector<std::vector <int>> of actions, for this case I'll just add the two I added.


	/*                Pathfinding Code ends                    */
	/***********************************************************/

	sf::RenderWindow window(sf::VideoMode(1010, 1010), "CAP4621 Project", sf::Style::Titlebar | sf::Style::Close);
	while (window.isOpen())
	{
		PathFunc(Map, Pathing, PathingCount);
		//The function above is called to start the pathfinding, nothing should be needed to be changed just what is contained
		//in the Actions vector.
		//This function will set Pathing[0] to false once it's done pathing then add a true to the end of the pathing vector
		//making Pathing[1] true so that if we run Path(Actions, Map, Player, Pathing, 1) that function will start pathing once
		//this function ends so that we can path multiple times.


		sf::Event evnt;
		while (window.pollEvent(evnt))
		{

			if (evnt.type == sf::Event::Closed)
			{
				window.close();
			}
		}

		//Clearing the window.
		window.clear();

		//Drawing the Tilemap if not in LoadingMapMode or StateSpaceMode.
		if (!(LoadingMapMode||StateSpaceMode||SeeCharacterPOV))
		{
			int Parser = 0;
			while (Parser < Map.TileVector.size())
			{
				int InnerParser = 0;
				while (InnerParser < Map.TileVector[Parser].size())
				{
					window.draw(Map.TileVector[Parser][InnerParser]);
					InnerParser++;
				}
				Parser++;
			}
		}
		if (SeeCharacterPOV && !(StateSpaceMode) && !(LoadingMapMode))
		{
			int Parser = 0;
			while (Parser < CharacterPOV.TileVector.size())
			{
				int InnerParser = 0;
				while (InnerParser < CharacterPOV.TileVector[Parser].size())
				{
					window.draw(CharacterPOV.TileVector[Parser][InnerParser]);
					InnerParser++;
				}
				Parser++;
			}
		}
		if (StateSpaceMode && !(LoadingMapMode))
		{
			int Parser = 0;
			while (Parser < Map.StateVector.size())
			{
				int InnerParser = 0;
				while (InnerParser < Map.StateVector[0].size())
				{
					window.draw(Map.StateVector[Parser][InnerParser]);
					InnerParser++;
				}
				Parser++;
			}
		}

		//I know it's technically inefficient to draw over the tilemap rather than replace the old one, but I'd have to save
		//the old one so I think this is fine.
		if (LoadingMapMode)
		{
			//During LoadingMapMode all other buttons but cancel, changing maps, and accept map are inactive.
			AddingMoves = false;
			CreateButtonPressed = false;
			SetCharacterPressed = false;
			AddingObstacles = false;
			RemoveToggled = false;
			MovingCharacter = false;
			std::vector<DataNode > CharData;
			CharData = LoadingMap.LoadMap(CurrentMap);
			if (CharData.size() != 0)
			{
				AgentYellow.SetLocationInt(CharData[0].CurrentLocation[0], CharData[0].CurrentLocation[1]);
				AgentYellow.GoalLocation[0] = CharData[0].GoalLocation[0];
				AgentYellow.GoalLocation[1] = CharData[0].GoalLocation[1];
				AgentYellow.DoDraw = CharData[0].Drawn;
				AgentYellow.Actions.clear();
				int ActionParser = 0;
				while (ActionParser < CharData[0].Actions.size())
				{
					AgentYellow.AddAction(CharData[0].Actions[ActionParser][0], CharData[0].Actions[ActionParser][1]);
					ActionParser++;
				}
				AgentGreen.SetLocationInt(CharData[1].CurrentLocation[0], CharData[1].CurrentLocation[1]);
				AgentGreen.GoalLocation[0] = CharData[1].GoalLocation[0];
				AgentGreen.GoalLocation[1] = CharData[1].GoalLocation[1];
				AgentGreen.DoDraw = CharData[1].Drawn;
				AgentGreen.Actions.clear();
				ActionParser = 0;
				while (ActionParser < CharData[1].Actions.size())
				{
					AgentGreen.AddAction(CharData[1].Actions[ActionParser][0], CharData[1].Actions[ActionParser][1]);
					ActionParser++;
				}
				AgentRed.SetLocationInt(CharData[2].CurrentLocation[0], CharData[2].CurrentLocation[1]);
				AgentRed.GoalLocation[0] = CharData[2].GoalLocation[0];
				AgentRed.GoalLocation[1] = CharData[2].GoalLocation[1];
				AgentRed.DoDraw = CharData[2].Drawn;
				AgentRed.Actions.clear();
				ActionParser = 0;
				while (ActionParser < CharData[2].Actions.size())
				{
					AgentRed.AddAction(CharData[2].Actions[ActionParser][0], CharData[2].Actions[ActionParser][1]);
					ActionParser++;
				}
			}
			if (CharData.size() != 0)
			{
				//First we draw the new map.
				int Parser = 0;
				while (Parser < LoadingMap.TileVector.size())
				{
					int InnerParser = 0;
					while (InnerParser < LoadingMap.TileVector[Parser].size())
					{
						window.draw(LoadingMap.TileVector[Parser][InnerParser]);
						InnerParser++;
					}
					Parser++;
				}
				//Then we let the player choose to change maps if they desire via buttons which we only draw in loading map mode.
			}
			else
			{
				//If the map failed to load we don't have a saved map at CurrentMap's value so we switch back to one (the start).
				CurrentMap = 1;
			}
			if (PressedRight)
			{
				CurrentMap++;
				std::this_thread::sleep_for(0.01s);
				PressedRight = false;
			}
			if (PressedLeft)
			{
				CurrentMap--;
				std::this_thread::sleep_for(0.01s);
				PressedLeft = false;
			}
			if (ChosenMap)
			{
				//If we select the map we remove our old map and set it to what we loaded in.
				Map = LoadingMap;
				LoadingMapMode = false;
				CurrentMap = 1;
				std::this_thread::sleep_for(0.01s);
				ChosenMap = false;
			}
			//Drawing the buttons.
			window.draw(RightArrowTile);
			window.draw(LeftArrowTile);
			window.draw(SelectMapTile);
			LoadingMapText.setString("Map: " + std::to_string(CurrentMap));
			window.draw(LoadingMapText);
			//Create events for loading map mode only.
			if (evnt.type == sf::Event::MouseButtonPressed)
			{
				if (evnt.mouseButton.button == sf::Mouse::Left)
				{
					if (LoadingMapMode)
					{
						sf::Vector2i MousePos = sf::Mouse::getPosition(window);
						//These positions indicate the right arrow was pressed.
						if (MousePos.x < 664 && MousePos.x >= 600 && MousePos.y < 976 && MousePos.y > 912)
						{
							//Increment CurrentMap to go to the next map.
							PressedRight = true;

						}
						//These positions indicate the left arrow was pressed.
						if (MousePos.x < 408 && MousePos.x >= 344 && MousePos.y < 976 && MousePos.y > 912)
						{
							PressedLeft = true;
						}
						//These positions indicate the select map button was pressed.
						if (MousePos.x < 536 && MousePos.x >= 472 && MousePos.y < 976 && MousePos.y > 912)
						{
							ChosenMap = true;
						}
					}
				}
			}
		}

		//Events that occur when a mouse button is pressed.
		if (evnt.type == sf::Event::MouseButtonPressed)
		{
			if (evnt.mouseButton.button == sf::Mouse::Left)
			{
				// DEBUGGING: Noting the position of the mouse std::cout << "Position: " << MousePos.x << " " << MousePos.y << std::endl;
				sf::Vector2i MousePos = sf::Mouse::getPosition(window);
				
				//Now we'll check if the MousePosition is on the grid (so between [120, 0] X [871, 911]).
				if (!((MousePos.x < 120) || (MousePos.x > 871) || (MousePos.y > 911)))
				{
					//If we're here our location is on the grid so we just set the location to the closest multiple of 16 for x and y.
					int xPos = floor((float)(MousePos.x - 9.0f) / 16.0f); //This will truncate so it rounds down as needed.
					xPos = xPos * 16;
					int yPos = floor((float)MousePos.y / 16.0f);
					yPos = yPos * 16;
					//Now that we have the position on the grid that the mouse pressed, we check what we're doing based on the booleans.
					//Which action we do is based on a priority system:
					//Creating Character > Setting character position > Adding moves to character > Adding obstacles to map > Removing target > Moving character
					if (CreateButtonPressed)
					{
						if(!(AgentYellow.DoDraw))
						{
							AgentYellow.DoDraw = true;
							AgentYellow.SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						}
						else if (!(AgentGreen.DoDraw))
						{
							AgentGreen.DoDraw = true;
							AgentGreen.SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						}
						else if (!(AgentRed.DoDraw))
						{
							AgentRed.DoDraw = true;
							AgentRed.SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						}
						else
						{
							AgentYellow.DoDraw = false;
							AgentGreen.DoDraw = false;
							AgentRed.DoDraw = false;
						}
						CreateButtonPressed = false;
					}
					else if (StateSpaceMode)
					{
						sf::Vector2f MousePos;
						MousePos.x = xPos + 1.0;
						MousePos.y = yPos - 8.0;
						int xLoc = xPos / 16 - 7;
						int yLoc = yPos / 16;
						bool AddedObstacle = false;
						if(Map.StateTraits[xLoc][yLoc][0])
						{
							Map.StateTraits[xLoc][yLoc][0] = false;
							sf::RectangleShape CurTile = Map.TileVector[xLoc][yLoc];
							CurTile.setTexture(&Map.StateSpaceTexture);
							Map.StateVector[xLoc][yLoc] = CurTile;
						}
						else
						{
							sf::RectangleShape ObstPlace = Map.TileVector[xLoc][yLoc];
							ObstPlace.setTexture(&Map.ObstacleTexture);
							Map.StateVector[xLoc][yLoc] = ObstPlace;
							Map.StateTraits[xLoc][yLoc][0] = true;
						}
						std::this_thread::sleep_for(0.2s);
					}
					else if (SetCharacterPressed)
					{
						TargetAgent->SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						SetCharacterPressed = false;
					}
					else if (SettingPath)
					{
						int xTargetPosition = xPos / 16 - 7;
						int yTargetPosition = yPos / 16;
						std::vector<int> GoalLoc;
						GoalLoc.push_back(xTargetPosition);
						GoalLoc.push_back(yTargetPosition);
						TargetAgent->GoalLocation = GoalLoc;
						SettingPath = false;
					}
					else if (AddingMoves)
					{
						sf::Vector2f NewAction;
						NewAction.x = xPos - TargetAgent->CurrentLocation.x;
						NewAction.y = yPos - TargetAgent->CurrentLocation.y;
						TargetAgent->Actions.push_back(NewAction);
						std::this_thread::sleep_for(0.2s);
					}
					else if (AddingObstacles)
					{
						sf::Vector2f ObsPosition;
						ObsPosition.x = xPos + 1.0;
						ObsPosition.y = yPos - 8.0;
						sf::RectangleShape ObstacleTile(sf::Vector2f(32.0, 32.0));
						ObstacleTile.setTexture(&ObstacleTileTexture);
						ObstacleTile.setPosition(ObsPosition);
						int xObsPos = xPos / 16;
						int yObsPos = yPos / 16;
						//Somehow xposition is off by 7 from map, moving it over.
						Map.TileVector[xObsPos - 7][yObsPos] = ObstacleTile;
						//Set the trait "isObstacle" to true.
						Map.SetTrait(xObsPos - 7, yObsPos, 0, true);
						
					}
					else if (RemoveToggled)
					{
						int xTargetPosition = xPos / 16 - 7;
						int yTargetPosition = yPos / 16;
						//First we'll check if we're on an obstacle, and if so remove it, to do that we'll need an obstacle tile.
						//First trait in tile traits is "isObstacle" so we just need where on the map it is.
						//First we check if its size is zero as if it is there's clearly no obstacle there otherwise it would've increased to add the isObstacle boolean.
						if (!(Map.TileTraits[xTargetPosition][yTargetPosition].size() == 0))
						{
							if (Map.TileTraits[xTargetPosition][yTargetPosition][0])
							{
								//If we're in here it's an obstacle, so now we remove it by setting it back to an original tile.
								Map.ResetTile(xTargetPosition, yTargetPosition);
							}
						}
						//Now we'll check this tile has an action on it, if so then the player's current position plus said action
						//should be equal to our xPos, yPos.
						int Parser = 0;
						while (Parser < TargetAgent->Actions.size())
						{
							int xAction = TargetAgent->CurrentLocation.x + TargetAgent->Actions[Parser].x;
							int yAction = TargetAgent->CurrentLocation.y + TargetAgent->Actions[Parser].y;
							if ((xPos == xAction) && (yPos == yAction))
							{
								//Now we know said action is the one we're targeting, so we remove it.
								TargetAgent->Actions.erase(TargetAgent->Actions.begin() + Parser);
							}
							Parser++;
						}

					}
					else if (MovingCharacter)
					{
						//We parse through the possible actions and check if our position is one such action.
						int Parser = 0;
						while (Parser < TargetAgent->Actions.size())
						{
							int xAction = TargetAgent->CurrentLocation.x + TargetAgent->Actions[Parser].x;
							int yAction = TargetAgent->CurrentLocation.y + TargetAgent->Actions[Parser].y;
							if ((xPos == xAction) && (yPos == yAction))
							{
								int xNewLoc = xPos / 16 - 7;
								int yNewLoc = yPos / 16;
								if (!(Map.TileTraits[xNewLoc][yNewLoc].size() == 0))
								{
									if(!(Map.TileTraits[xNewLoc][yNewLoc][0]))
									{
										TargetAgent->SetLocationInt(xNewLoc, yNewLoc);
									}
								}
								else
								{
									TargetAgent->SetLocationInt(xNewLoc, yNewLoc);
								}
								Parser = TargetAgent->Actions.size();
								//The sleep is in case the action we click on sends our mouse into another action.
								std::this_thread::sleep_for(0.2s);
							}
							Parser++;
						}
					}
					//Now that we've made the character and set it to be drawn we'll set CreateButtonPressed to false as the character is made.
				}
				//These positions indicate the create button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 65 && MousePos.y >= 0)
				{
					CreateButtonPressed = true;
				}
				//These positions indicate the set character button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 129 && MousePos.y > 65)
				{
					SetCharacterPressed = true;
				}
				//These positions indicate the toggle moves button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 257 && MousePos.y > 193)
				{
					if (AddingMoves)
					{
						AddingMoves = false;
					}
					else
					{
						AddingMoves = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the toggle obstacles button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 321 && MousePos.y > 257)
				{
					if (AddingObstacles)
					{
						AddingObstacles = false;
					}
					else
					{
						AddingObstacles = true;
					}
					std::vector<std::vector<int> > Testing = TargetAgent->GetActions();
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the remove toggle button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 385 && MousePos.y > 321)
				{
					if (RemoveToggled)
					{
						RemoveToggled = false;
					}
					else
					{
						RemoveToggled = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the moving character toggle button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 449 && MousePos.y > 385)
				{
					if (MovingCharacter)
					{
						MovingCharacter = false;
					}
					else
					{
						MovingCharacter = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the set path button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 513 && MousePos.y > 449)
				{
					if (SettingPath)
					{
						SettingPath = false;
					}
					else
					{
						SettingPath = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the save map button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 636 && MousePos.y > 572)
				{
					std::cout << "Saving Map" << std::endl;
					std::vector<DataNode> AgentList;
					DataNode YellowNode;
					YellowNode.Drawn = AgentYellow.DoDraw;
					YellowNode.CurrentLocation = AgentYellow.GetLocation();
					YellowNode.GoalLocation.push_back(AgentYellow.GoalLocation[0]);
					YellowNode.GoalLocation.push_back(AgentYellow.GoalLocation[1]);
					YellowNode.Actions = AgentYellow.GetActions();
					AgentList.push_back(YellowNode);
					DataNode GreenNode;
					GreenNode.Drawn = AgentGreen.DoDraw;
					GreenNode.CurrentLocation = AgentGreen.GetLocation();
					GreenNode.GoalLocation.push_back(AgentGreen.GoalLocation[0]);
					GreenNode.GoalLocation.push_back(AgentGreen.GoalLocation[1]);
					GreenNode.Actions = AgentGreen.GetActions();
					AgentList.push_back(GreenNode);
					DataNode RedNode;
					RedNode.Drawn = AgentRed.DoDraw;
					RedNode.CurrentLocation = AgentRed.GetLocation();
					RedNode.GoalLocation.push_back(AgentRed.GoalLocation[0]);
					RedNode.GoalLocation.push_back(AgentRed.GoalLocation[1]);
					RedNode.Actions = AgentRed.GetActions();
					AgentList.push_back(RedNode);
					Map.SaveMap(AgentList);
					std::this_thread::sleep_for(0.2s);
					SavedMap = true;
				}
				//These positions indicate the load map button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 700 && MousePos.y > 636)
				{
					std::cout << "Load map pressed, entering load state." << std::endl;
					std::this_thread::sleep_for(0.2s);
					//Since we're entering loading map mode we store the current characters in case we cancel.
					AgentPreserver.push_back(AgentYellow);
					AgentPreserver.push_back(AgentGreen);
					AgentPreserver.push_back(AgentRed);
					LoadingMapMode = true;
				}
				//These positions indicate the cancel button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 912 && MousePos.y > 848)
				{
					//Cancel sets all the above booleans to false so our mouse isn't pending any actions.
					AddingMoves = false;
					CreateButtonPressed = false;
					SetCharacterPressed = false;
					AddingObstacles = false;
					RemoveToggled = false;
					MovingCharacter = false;
					SettingPath = false;
					if (LoadingMapMode)
					{
						std::cout << "Cancel pressed, exiting load state.";
						AgentYellow = AgentPreserver[0];
						AgentGreen = AgentPreserver[1];
						AgentRed = AgentPreserver[2];
					}
					LoadingMapMode = false;
				}
				//These positions indicate the yellow character was selected
				if (MousePos.x < 1006 && MousePos.x >= 976 && MousePos.y > 128 && MousePos.y <= 160)
				{
					
					TargetAgent = &AgentYellow;
					CurrentTargetTile.setTexture(&YellowTexture);
				}
				//These positions indicate the green character was selected
				if (MousePos.x < 1006 && MousePos.x >= 976 && MousePos.y > 152 && MousePos.y <= 184)
				{
					
					TargetAgent = &AgentGreen;
					CurrentTargetTile.setTexture(&GreenTexture);
				}
				//These positions indicate the red character was selected
				if (MousePos.x < 1006 && MousePos.x >= 976 && MousePos.y > 176 && MousePos.y <= 208)
				{
					
					TargetAgent = &AgentRed;
					CurrentTargetTile.setTexture(&RedTexture);
				}
				//These positions indicate the path button was pressed.
				if (MousePos.x < 1008 && MousePos.x >= 944 && MousePos.y > 224 && MousePos.y <= 288)
				{
					TargetAgent->radius = 3;

					std::vector<std::vector<std::vector<bool>>> maskedObstacles = Map.TileTraits;
					for (int col = 0; col < maskedObstacles.size(); col++)
						for (int row = 0; row < maskedObstacles[0].size(); row++)
							if (maskedObstacles[col][row].size() != 0 && (abs(col - TargetAgent->GetLocation()[0]) > TargetAgent->radius || abs(row - TargetAgent->GetLocation()[1]) > TargetAgent->radius))
								maskedObstacles[col][row][0] = false;

					//Yellow will be AStar, Green LPAStar.
					if (TargetAgent->AgentType == 1)
					{
						TargetAgent->pfa = new AStar(TargetAgent->GetActions(), maskedObstacles);
					}
					if (TargetAgent->AgentType == 2)
					{
						TargetAgent->pfa = new LPA(TargetAgent->GetActions(), maskedObstacles);
					}
					PathReturn ResultPath = TargetAgent->pfa->Update(TargetAgent->GetActions(), maskedObstacles, TargetAgent->GetLocation(), TargetAgent->GoalLocation);
					std::vector<std::vector<int> > Actions = ResultPath.path;
					ActionsPerAgent.push_back(Actions);
					Agents.push_back(TargetAgent);
					PathingCount++;
				}
				//These positions indicate the path all button was pressed.
				if (MousePos.x < 1008 && MousePos.x >= 944 && MousePos.y > 288 && MousePos.y <= 352)
				{
					bool AtLeastOne = false;
					if (AgentYellow.DoDraw)
					{
						AgentYellow.radius = 3;

						std::vector<std::vector<std::vector<bool>>> maskedObstacles = Map.TileTraits;
						for (int col = 0; col < maskedObstacles.size(); col++)
							for (int row = 0; row < maskedObstacles[0].size(); row++)
								if (maskedObstacles[col][row].size() != 0 && (abs(col - AgentYellow.GetLocation()[0]) > AgentYellow.radius || abs(row - AgentYellow.GetLocation()[1]) > AgentYellow.radius))
									maskedObstacles[col][row][0] = false;

						AgentYellow.pfa = new DStar(AgentYellow.GetActions(), maskedObstacles);
						PathReturn ResultPath = AgentYellow.pfa->Update(AgentYellow.GetActions(), maskedObstacles, AgentYellow.GetLocation(), AgentYellow.GoalLocation);
						std::vector<std::vector<int> > Actions = ResultPath.path;
						ActionsPerAgent.push_back(Actions);
						Agents.push_back(&AgentYellow);
						AtLeastOne = true;
					}
					if (AgentGreen.DoDraw)
					{
						AgentGreen.radius = 3;

						std::vector<std::vector<std::vector<bool>>> maskedObstacles = Map.TileTraits;
						for (int col = 0; col < maskedObstacles.size(); col++)
							for (int row = 0; row < maskedObstacles[0].size(); row++)
								if (maskedObstacles[col][row].size() != 0 && (abs(col - AgentYellow.GetLocation()[0]) > AgentYellow.radius || abs(row - AgentYellow.GetLocation()[1]) > AgentYellow.radius))
									maskedObstacles[col][row][0] = false;

						AgentGreen.pfa = new LPA(AgentGreen.GetActions(), maskedObstacles);
						PathReturn ResultPath = AgentGreen.pfa->Update(AgentGreen.GetActions(), maskedObstacles, AgentGreen.GetLocation(), AgentGreen.GoalLocation);
						std::vector<std::vector<int> > Actions = ResultPath.path;
						ActionsPerAgent.push_back(Actions);
						Agents.push_back(&AgentGreen);
						AtLeastOne = true;
					}
					if (AgentRed.DoDraw)
					{
						AgentRed.radius = 3;

						std::vector<std::vector<std::vector<bool>>> maskedObstacles = Map.TileTraits;
						for (int col = 0; col < maskedObstacles.size(); col++)
							for (int row = 0; row < maskedObstacles[0].size(); row++)
								if (maskedObstacles[col][row].size() != 0 && (abs(col - AgentYellow.GetLocation()[0]) > AgentYellow.radius || abs(row - AgentYellow.GetLocation()[1]) > AgentYellow.radius))
									maskedObstacles[col][row][0] = false;

						AgentRed.pfa = new AStar(AgentRed.GetActions(), maskedObstacles);
						PathReturn ResultPath = AgentRed.pfa->Update(AgentRed.GetActions(), maskedObstacles, AgentRed.GetLocation(), AgentRed.GoalLocation);
						std::vector<std::vector<int> > Actions = ResultPath.path;
						ActionsPerAgent.push_back(Actions);
						Agents.push_back(&AgentRed);
						AtLeastOne = true;
					}
					if (AtLeastOne)
					{
						PathingCount++;
					}
				}
				//These positions indicate the State Space button was pressed.
				if (MousePos.x < 1008 && MousePos.x >= 944 && MousePos.y > 352 && MousePos.y <= 416)
				{
					if (StateSpaceMode)
					{
						StateSpaceMode = false;
					}
					else
					{
						StateSpaceMode = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
				//These positions indicate the CharPOV button was pressed
				if (MousePos.x < 1008 && MousePos.x >= 944 && MousePos.y > 480 && MousePos.y <= 544)
				{
					if (SeeCharacterPOV)
					{
						SeeCharacterPOV = false;
					}
					else
					{
						SeeCharacterPOV = true;
					}
					std::this_thread::sleep_for(0.2s);
				}
			}
		}

		//Same priority as above.
		if (CreateButtonPressed)
		{
			Text.setString("Please select a location on the grid of blue squares.");
			window.draw(Text);
		}

		if (SavedMap)
		{
			std::cout << "Map saved!" << std::endl;
			ToggleIndicatorText.setString("Map Saved!");
			window.draw(ToggleIndicatorText);
			//Note: This turns itself off so the text can't really be seen, for now put in console.
			SavedMap = false;
		}
		else if (SettingPath)
		{
			ToggleIndicatorText.setString("Select goal location");
			window.draw(ToggleIndicatorText);
		}
		else if (AddingMoves)
		{
			ToggleIndicatorText.setString("Currently adding moves");
			window.draw(ToggleIndicatorText);
		}
		else if (AddingObstacles)
		{
			ToggleIndicatorText.setString("Currently adding obstacles");
			window.draw(ToggleIndicatorText);
		}
		else if (RemoveToggled)
		{
			ToggleIndicatorText.setString("Currently removing non-player targets");
			window.draw(ToggleIndicatorText);
		}
		else if (MovingCharacter)
		{
			ToggleIndicatorText.setString("Currently moving character");
			window.draw(ToggleIndicatorText);
		}
		else if (SeeCharacterPOV)
		{
			ToggleIndicatorText.setString("Viewing Character POV");
			window.draw(ToggleIndicatorText);
		}

		//Drawing UI (the buttons)--drawn in the order they appear top down.
		window.draw(CreateButtonTile);
		window.draw(SetCharacterTile);
		window.draw(ToggleMovesTile);
		window.draw(ToggleObstaclesTile);
		window.draw(RemoveToggleTile);
		window.draw(MoveCharacterTile);
		window.draw(SetPathTile);
		window.draw(SaveMapTile);
		window.draw(LoadMapTile);
		window.draw(CancelButtonTile);

		//Drawing the character select UI
		window.draw(SetTargetTile);
		window.draw(CurrentTargetTile);
		window.draw(AgentYellowTile);
		window.draw(AgentGreenTile);
		window.draw(AgentRedTile);
		
		//Drawing the UI buttons on the right side.
		window.draw(PathTile);
		window.draw(PathAllTile);
		window.draw(StateSpaceTile);
		/*Drawing auto pather*/
		window.draw(AutoPatherTile);
		/*Auto pather code here over*/
		window.draw(CharPOVTile);

		//Drawing characters.
		if ((AgentYellow.DoDraw))
		{
			window.draw(AgentYellow.CharacterTile);
			//From there we draw a lighter square for any locations the player can move to.
			sf::RectangleShape GoalTile(sf::Vector2f(32.0, 32.0));
			if (AgentYellow.GoalLocation.size() == 2)
			{
				GoalTile.setTexture(&YellowTexture);
				int xGoalInt = AgentYellow.GoalLocation[0];
				int yGoalInt = AgentYellow.GoalLocation[1];
				float xGoalLoc = (xGoalInt + 7) * 16 + 1;
				float yGoalLoc = (yGoalInt * 16) - 8;
				GoalTile.setPosition(sf::Vector2f(xGoalLoc, yGoalLoc));
				bool Obst = false;
				if (!(Map.TileTraits[xGoalInt][yGoalInt].size() == 0))
				{
					if (Map.TileTraits[xGoalInt][yGoalInt][0])
					{
						Obst = true;
					}
				}
				if (!(Obst))
				{
					window.draw(GoalTile);
				}
			}
			int Parser = 0;
			while (Parser < AgentYellow.Actions.size())
			{
				sf::RectangleShape ActionTile(sf::Vector2f(32.0, 32.0));
				ActionTile.setTexture(&ActionTileTexture);
				//Since each action is a vector, the location would be the player's location, plus the vector, times 16 to convert to pixels.
				float xLoc = AgentYellow.CurrentLocation.x + AgentYellow.Actions[Parser].x + 1.0;
				float yLoc = AgentYellow.CurrentLocation.y + AgentYellow.Actions[Parser].y - 8.0;
				ActionTile.setPosition(sf::Vector2f(xLoc, yLoc));
				//Also get its tile location on the minimap.
				int xTileLoc = (AgentYellow.CurrentLocation.x + AgentYellow.Actions[Parser].x) / 16 - 7;
				int yTileLoc = (AgentYellow.CurrentLocation.y + AgentYellow.Actions[Parser].y) / 16;
				//Test if the action sends us to a position on the map, if not don't draw it.
				if (!((xTileLoc < 0) || (xTileLoc > 46) || (yTileLoc > 56) || (yTileLoc < 0)))
				{
					//If it's on the map, also test if the location is where an obstacle is.
					bool NoObstacle = true;
					if (!(Map.TileTraits[xTileLoc][yTileLoc].size() == 0))
					{
						if (Map.TileTraits[xTileLoc][yTileLoc][0])
						{
							NoObstacle = false;
						}
					}
					if (NoObstacle)
					{
						window.draw(ActionTile);
					}
				}
				Parser++;
			}
		}
		//Drawing agent green.
		if ((AgentGreen.DoDraw))
		{
			window.draw(AgentGreen.CharacterTile);
			//From there we draw a lighter square for any locations the player can move to.
			sf::RectangleShape GoalTile(sf::Vector2f(32.0, 32.0));
			if (AgentGreen.GoalLocation.size() == 2)
			{
				GoalTile.setTexture(&GreenTexture);
				int xGoalInt = AgentGreen.GoalLocation[0];
				int yGoalInt = AgentGreen.GoalLocation[1];
				float xGoalLoc = (xGoalInt + 7) * 16 + 1;
				float yGoalLoc = (yGoalInt * 16) - 8;
				GoalTile.setPosition(sf::Vector2f(xGoalLoc, yGoalLoc));
				bool Obst = false;
				if (!(Map.TileTraits[xGoalInt][yGoalInt].size() == 0))
				{
					if (Map.TileTraits[xGoalInt][yGoalInt][0])
					{
						Obst = true;
					}
				}
				if (!(Obst))
				{
					window.draw(GoalTile);
				}
			}
			int Parser = 0;
			while (Parser < AgentGreen.Actions.size())
			{
				sf::RectangleShape ActionTile(sf::Vector2f(32.0, 32.0));
				ActionTile.setTexture(&ActionTileTexture);
				//Since each action is a vector, the location would be the player's location, plus the vector, times 16 to convert to pixels.
				float xLoc = AgentGreen.CurrentLocation.x + AgentGreen.Actions[Parser].x + 1.0;
				float yLoc = AgentGreen.CurrentLocation.y + AgentGreen.Actions[Parser].y - 8;
				ActionTile.setPosition(sf::Vector2f(xLoc, yLoc));
				//Also get its tile location on the minimap.
				int xTileLoc = (AgentGreen.CurrentLocation.x + AgentGreen.Actions[Parser].x) / 16 - 7;
				int yTileLoc = (AgentGreen.CurrentLocation.y + AgentGreen.Actions[Parser].y) / 16;
				//Test if the action sends us to a position on the map, if not don't draw it.
				if (!((xTileLoc < 0) || (xTileLoc > 46) || (yTileLoc > 56) || (yTileLoc < 0)))
				{
					//If it's on the map, also test if the location is where an obstacle is.
					bool NoObstacle = true;
					if (!(Map.TileTraits[xTileLoc][yTileLoc].size() == 0))
					{
						if (Map.TileTraits[xTileLoc][yTileLoc][0])
						{
							NoObstacle = false;
						}
					}
					if (NoObstacle)
					{
						window.draw(ActionTile);
					}
				}
				Parser++;
			}
		}
		//Drawing agent red.
		if ((AgentRed.DoDraw))
		{
			window.draw(AgentRed.CharacterTile);
			//From there we draw a lighter square for any locations the player can move to.
			sf::RectangleShape GoalTile(sf::Vector2f(32.0, 32.0));
			if (AgentRed.GoalLocation.size() == 2)
			{
				GoalTile.setTexture(&RedTexture);
				int xGoalInt = AgentRed.GoalLocation[0];
				int yGoalInt = AgentRed.GoalLocation[1];
				float xGoalLoc = (xGoalInt + 7) * 16 + 1;
				float yGoalLoc = (yGoalInt * 16) - 8;
				GoalTile.setPosition(sf::Vector2f(xGoalLoc, yGoalLoc));
				bool Obst = false;
				if (!(Map.TileTraits[xGoalInt][yGoalInt].size() == 0))
				{
					if (Map.TileTraits[xGoalInt][yGoalInt][0])
					{
						Obst = true;
					}
				}
				if (!(Obst))
				{
					window.draw(GoalTile);
				}
			}
			int Parser = 0;
			while (Parser < AgentRed.Actions.size())
			{
				sf::RectangleShape ActionTile(sf::Vector2f(32.0, 32.0));
				ActionTile.setTexture(&ActionTileTexture);
				//Since each action is a vector, the location would be the player's location, plus the vector, times 16 to convert to pixels.
				float xLoc = AgentRed.CurrentLocation.x + AgentRed.Actions[Parser].x + 1.0;
				float yLoc = AgentRed.CurrentLocation.y + AgentRed.Actions[Parser].y - 8.0;
				ActionTile.setPosition(sf::Vector2f(xLoc, yLoc));
				//Also get its tile location on the minimap.
				int xTileLoc = (AgentRed.CurrentLocation.x + AgentRed.Actions[Parser].x) / 16 - 7;
				int yTileLoc = (AgentRed.CurrentLocation.y + AgentRed.Actions[Parser].y) / 16;
				//Test if the action sends us to a position on the map, if not don't draw it.
				if (!((xTileLoc < 0) || (xTileLoc > 46) || (yTileLoc > 56) || (yTileLoc < 0)))
				{
					//If it's on the map, also test if the location is where an obstacle is.
					bool NoObstacle = true;
					if (!(Map.TileTraits[xTileLoc][yTileLoc].size() == 0))
					{
						if (Map.TileTraits[xTileLoc][yTileLoc][0])
						{
							NoObstacle = false;
						}
					}
					if (NoObstacle)
					{
						window.draw(ActionTile);
					}
				}
				Parser++;
			}
		}

		window.display();
	}

	return 0;
}