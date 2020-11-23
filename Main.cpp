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

void Path(std::vector<std::vector<int> > PathActions, TileMap& Map, Character& Player, std::vector<bool>& Pathing, int PathCount)
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
	else if ((CurrentlyPathing)&&(PathActions.size() > 0))
	{
		//We get our current action, which is PathActions for at PathCounter.
		std::vector<int> CurrentAction = PathActions[PathCounter];
		Player.Move(CurrentAction[0], CurrentAction[1], Map);
		PathCounter++;
		if (!(PathCounter < PathActions.size()))
		{
			Pathing[PathCount] = false; //Set the Pathing boolean for Path PathCount (so the first Path if PathCount == 0) to false as the path is done.
			if (Pathing.size() == PathCount + 1) //If this is the case then we need to push_back into our next path (so we can add another later).
			{
				Pathing.push_back(true); //Push back a true as it is our next path adn it is the one we're on.
			}
			else
			{
				Pathing[PathCount + 1] = true; //Otherwise just set the next one equal to true.
			}
			PathCounter = -1;
		}
		std::this_thread::sleep_for(1s); //Sleeping so we visually see the change.
	}
};

int main()
{

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

	//Creating the character object.
	Character Player("images/Yellow_Square_Black_Border_16_16.png");

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
	Map.LoadMap(7);
	//To check if there's an obstacle use Map.IsObstacle(int xLoc, int yLoc), for example I know that at (21, 5) there's an obstacle.
	//**Note: I count from 0 here for this, think of a 2 dimensional array for the grid**
	std::cout << Map.IsObstacle(21, 5) << std::endl;
	Player.DoDraw = true; //Set Player.DoDraw to true so it's seen without needing to hit create character.
	Player.SetLocationInt(5, 5);
	std::vector<int> CurrentLoc = Player.GetLocation();
	std::cout << CurrentLoc[0] << " " << CurrentLoc[1] << std::endl;
	std::this_thread::sleep_for(1s);
	//Note that Move is used in the Path() function that paths for you, if it runs into an illegal move it just doesn't do the
	//move and continues on (I think, haven't tested that for 100% certainty)
	Player.Move(2, 2, Map);
	Player.Move(14, 18, Map);
	CurrentLoc = Player.GetLocation();
	std::cout << CurrentLoc[0] << " " << CurrentLoc[1] << std::endl;
	Player.AddAction(0, 1);
	Player.AddAction(1, 0);
	Player.AddAction(0, -1);
	Player.AddAction(-1, 0);
	PathFinder* pathPlanner = new LPA(Player.GetActions(), Map.TileTraits);
	std::vector<int> TestVect;
	TestVect.push_back(5);
	TestVect.push_back(23);
	PathReturn* path = pathPlanner->Update(Player.GetActions(), Map.TileTraits, Player.GetLocation(), TestVect);
	std::vector<std::vector<int>> Actions = path->path;
	int Parser = 0;
	while (Parser < Actions.size())
	{
		std::cout << Actions[Parser][0] << " " << Actions[Parser][1] << std::endl;
		Parser++;
	}
	//I currently haven't created a way to save characters, so actions need to be added manually using Player.AddAction(int x, int y).
	//For the path (note: see code up top for how it works: design an std::vector<std::vector <int>> of actions, for this case I'll just add the two I added.


	/*                Pathfinding Code ends                    */
	/***********************************************************/

	sf::RenderWindow window(sf::VideoMode(1010, 1010), "CAP4621 Project", sf::Style::Titlebar | sf::Style::Close);
	while (window.isOpen())
	{
		Path(Actions, Map, Player, Pathing, 0);
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

		//Drawing the Tilemap if not in LoadingMapMode.
		if (!(LoadingMapMode))
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
			bool MapLoaded = LoadingMap.LoadMap(CurrentMap);
			if (MapLoaded)
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
							CurrentMap++;
							std::this_thread::sleep_for(0.2s);
						}
						//These positions indicate the left arrow was pressed.
						if (MousePos.x < 408 && MousePos.x >= 344 && MousePos.y < 976 && MousePos.y > 912)
						{
							CurrentMap--;
							std::this_thread::sleep_for(0.2s);
						}
						//These positions indicate the select map button was pressed.
						if (MousePos.x < 536 && MousePos.x >= 472 && MousePos.y < 976 && MousePos.y > 912)
						{
							//If we select the map we remove our old map and set it to what we loaded in.
							Map = LoadingMap;
							LoadingMapMode = false;
							CurrentMap = 1;
							std::this_thread::sleep_for(0.2s);
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
						Player.SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						//Remove all actions as this is a new character.
						Player.Actions.clear();
						Player.DoDraw = true;
						CreateButtonPressed = false;
					}
					else if (SetCharacterPressed)
					{
						Player.SetLocation(sf::Vector2f((float)xPos + 1.0, (float)yPos - 8.0));
						SetCharacterPressed = false;
					}
					else if (AddingMoves)
					{
						sf::Vector2f NewAction;
						NewAction.x = xPos - Player.CurrentLocation.x;
						NewAction.y = yPos - Player.CurrentLocation.y;
						Player.Actions.push_back(NewAction);
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
						while (Parser < Player.Actions.size())
						{
							int xAction = Player.CurrentLocation.x + Player.Actions[Parser].x;
							int yAction = Player.CurrentLocation.y + Player.Actions[Parser].y;
							if ((xPos == xAction) && (yPos == yAction))
							{
								//Now we know said action is the one we're targeting, so we remove it.
								Player.Actions.erase(Player.Actions.begin() + Parser);
							}
							Parser++;
						}

					}
					else if (MovingCharacter)
					{
						//We parse through the possible actions and check if our position is one such action.
						int Parser = 0;
						while (Parser < Player.Actions.size())
						{
							int xAction = Player.CurrentLocation.x + Player.Actions[Parser].x;
							int yAction = Player.CurrentLocation.y + Player.Actions[Parser].y;
							if ((xPos == xAction) && (yPos == yAction))
							{
								int xNewLoc = xPos / 16 - 7;
								int yNewLoc = yPos / 16;
								Player.Move(xNewLoc, yNewLoc, Map);
								Parser = Player.Actions.size();
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
					std::vector<std::vector<int> > Testing = Player.GetActions();
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
				//These positions indicate the save map button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 636 && MousePos.y > 572)
				{
					std::cout << "Saving Map" << std::endl;
					Map.SaveMap();
					std::this_thread::sleep_for(0.2s);
					SavedMap = true;
				}
				//These positions indicate the load map button was pressed.
				if (MousePos.x < 65 && MousePos.x >= 0 && MousePos.y < 700 && MousePos.y > 636)
				{
					std::cout << "Load map pressed, entering load state." << std::endl;
					std::this_thread::sleep_for(0.2s);
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
					if (LoadingMapMode)
					{
						std::cout << "Cancel pressed, exiting load state.";
					}
					LoadingMapMode = false;
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

		//Drawing UI (the buttons)--drawn in the order they appear top down.
		window.draw(CreateButtonTile);
		window.draw(SetCharacterTile);
		window.draw(ToggleMovesTile);
		window.draw(ToggleObstaclesTile);
		window.draw(RemoveToggleTile);
		window.draw(MoveCharacterTile);
		window.draw(SaveMapTile);
		window.draw(LoadMapTile);
		window.draw(CancelButtonTile);

		//Drawing characters.
		if ((Player.DoDraw)&&(!(LoadingMapMode)))
		{
			window.draw(Player.CharacterTile);
			//From there we draw a lighter square for any locations the player can move to.
			int Parser = 0;
			while (Parser < Player.Actions.size())
			{
				sf::RectangleShape ActionTile(sf::Vector2f(32.0, 32.0));
				ActionTile.setTexture(&ActionTileTexture);
				//Since each action is a vector, the location would be the player's location, plus the vector, times 16 to convert to pixels.
				float xLoc = Player.CurrentLocation.x + Player.Actions[Parser].x + 1.0;
				float yLoc = Player.CurrentLocation.y + Player.Actions[Parser].y - 8.0;
				ActionTile.setPosition(sf::Vector2f(xLoc, yLoc));
				//Also get its tile location on the minimap.
				int xTileLoc = (Player.CurrentLocation.x + Player.Actions[Parser].x) / 16 - 7;
				int yTileLoc = (Player.CurrentLocation.y + Player.Actions[Parser].y) / 16;
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