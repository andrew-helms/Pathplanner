This is a quick rundown of the features of the UI, as for coding and how you'll use it that's at the end of the file (labled accordingly).

Create -- creates a character, removes the actions of the previous character and creates it at the position on the map you select.
SetChar -- Sets the characters position to be where on the map you select regardless of whether it's a legal move.
ToggleAddMoves -- Causes your mouse to add moves to the character, only relevant if the character is already created.
ToggleObstacles -- Causes your mouse to craete obstacles.
Remove -- Toggles removing obstacles and moves, removes the obstacle or move on the point you select, both if both are present (I think, didn't test that).
Move Character -- Toggles moving the character, causing you to be able to move the character by selecting on legal move points indicated in orange.

Save Map -- saves the current map (not character, just where the obstacles are atm) to the SavedMaps file
*Note: the naming convention of Map1, Map2, etc. is very important, if they aren't in order from 1 - however many maps (so you jump from 1 to 3) the system will
fail, if you delete a map rename to reorder accordingly.*
Load Map -- loads the maps saved in the SavedMaps file via taking advantage of the naming convention.
Once you hit Load map you (as the console notes) enter a loadmap state, all other buttons save for SaveMap, cancel, and the new RightArrow, LeftArrow, and
Select Map button are disabled, Right Arrow cycles forward through maps, Left Arrow backward, Select Map sets your map to the one currently displayed, cancel
returns you to your previous map before you hit load map.

There currently is no save character feature. 

Now for how you can use this: Go to Main.cpp, scroll down until you find a section called "Pathfinding Code in Main Here", implementation is 
described there along with a simple example of creating a path.