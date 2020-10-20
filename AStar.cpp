#include "AStar.h"
#include <iostream>

AStar::AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles)
{
	obstacles = Obstacles;
	actionSpace = actions;
}

std::vector<std::vector<int>> AStar::findPath(std::vector<int> start, std::vector<int> goal)
{
	std::vector<std::vector<int>> path;
	Coordinate* startCoord = new Coordinate(start[0], start[1]);
	Coordinate* goalCoord = new Coordinate(goal[0], goal[1]);
	stateSpace[*startCoord] = new Node(startCoord, nullptr, 0, start); //adds start as action from the parent just as a placeholder. Not used
	std::vector<Node*> queue;
	queue.push_back(stateSpace[*startCoord]);

	while (!queue.empty())
	{
		int minNode = 0;

		for (int i = 0; i < queue.size(); i++)
			if (queue[i]->cost + Heuristic(*queue[i]->getPosition(), *goalCoord) < queue[minNode]->cost + Heuristic(*queue[minNode]->getPosition(), *goalCoord))
				minNode = i;

		Node* curr = queue[minNode];
		queue.erase(queue.begin() + minNode);

		if (*curr->getPosition() == *goalCoord)
		{
			path.push_back(curr->actionFromParent);
			
			while (curr->parent != nullptr)
			{
				path.push_back(curr->actionFromParent);
				curr = curr->parent;
			}
		}

		for (int i = 0; i < actionSpace.size(); i++)
		{
			Coordinate* newCoord = new Coordinate(curr->getPosition()->x + actionSpace[i][0], curr->getPosition()->y + actionSpace[i][1]);

			if (newCoord->x >= obstacles.size() || newCoord->x < 0 || newCoord->y >= obstacles[0].size() || newCoord->y < 0 || obstacles[newCoord->x][newCoord->y].size() != 0 && obstacles[newCoord->x][newCoord->y][0])
			{
				delete newCoord;
				continue;
			}

			if (stateSpace.count(*newCoord) == 0)
			{
				Node* child = new Node(newCoord, curr, curr->cost + 1, actionSpace[i]); //Each move currently only has a cost of 1
				stateSpace[*newCoord] = child;
				queue.push_back(child);
			}
			else 
			{
				Node* temp = stateSpace[*newCoord];

				if (curr->cost + 1 < temp->cost)
				{
					temp->cost = curr->cost + 1;
					temp->actionFromParent = actionSpace[i];
					temp->parent = curr;
				}

				delete newCoord;
			}
		}
	}

	std::vector<std::vector<int>> outputPath;

	while (!path.empty())
	{
		outputPath.push_back(path.back());
		path.pop_back();
	}

	return outputPath;
}

int AStar::Heuristic(Coordinate Start, Coordinate End)
{
	return sqrt(pow(Start.x - End.x, 2) + pow(Start.y - End.y, 2));
}

Node::Node(Coordinate* Position, Node* Parent, int Cost, std::vector<int> ActionFromParent)
{
	position = Position;
	parent = Parent;
	cost = Cost;
	actionFromParent = ActionFromParent;
}

Coordinate* Node::getPosition()
{
	return position;
}

Coordinate::Coordinate(int X, int Y)
{
	x = X;
	y = Y;
}

bool operator==(const Coordinate& lhs, const Coordinate& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator<(const Coordinate& lhs, const Coordinate& rhs)
{
	return lhs.x + lhs.y < rhs.x + rhs.y;
}