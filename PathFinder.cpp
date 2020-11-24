#include "PathFinder.h"
#include <iostream>

AStar::AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : PathFinder(actions, Obstacles)
{}

PathReturn AStar::Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal)
{
	time_t startTime = time(NULL);

	Coordinate* startCoord = new Coordinate(start[0], start[1]);
	Coordinate* goalCoord = new Coordinate(goal[0], goal[1]);

	bool haveEdgesChanged = false;

	for (int col = 0; col < Obstacles.size(); col++)
	{
		for (int row = 0; row < Obstacles[0].size(); row++)
		{
			if (Obstacles[col][row].size() != 0 && obstacles[col][row].size() != 0 && Obstacles[col][row][0] != obstacles[col][row][0])
				haveEdgesChanged = true;
			else if ((Obstacles[col][row].size() != 0) != (obstacles[col][row].size() != 0))
				haveEdgesChanged = true;
		}
	}

	if (!firstRun && !haveEdgesChanged)
	{
		time_t endTime = time(NULL);

		exeTime += double(endTime - startTime);

		output.exeTime = exeTime;

		output.path.erase(output.path.begin());

		return output;
	}

	firstRun = false;

	actionSpace = actions;
	obstacles = Obstacles;

	stateSpace.clear();
	std::vector<std::vector<int>> path;
	stateSpace[*startCoord] = new Node(startCoord, nullptr, 0, start); //adds start as action from the parent just as a placeholder. Not used
	std::vector<Node*> queue;
	queue.push_back(stateSpace[*startCoord]);
	nodesExpanded++;
	 
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
			//path.push_back(curr->actionFromParent);
			
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
				Node* child = new Node(newCoord, curr, curr->cost + Heuristic(*curr->getPosition(), *newCoord), actionSpace[i]); //Each move currently only has a cost of 1
				stateSpace[*newCoord] = child;
				queue.push_back(child);
				nodesExpanded++;
			}
			else 
			{
				Node* temp = stateSpace[*newCoord];

				if (curr->cost + Heuristic(*curr->getPosition(), *newCoord) < temp->cost)
				{
					temp->cost = curr->cost + Heuristic(*curr->getPosition(), *newCoord);
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

	time_t endTime = time(NULL);

	exeTime += double(endTime - startTime);

	output = PathReturn(outputPath, nodesExpanded, exeTime);

	return output;
}

LPA::LPA(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : PathFinder(actions, Obstacles)
{}

PathReturn LPA::Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal)
{
	time_t startTime = time(NULL);

	Coordinate* startCoord = new Coordinate(start[0], start[1]);
	Coordinate* goalCoord = new Coordinate(goal[0], goal[1]);

	bool haveEdgesChanged = false;

	for (int col = 0; col < Obstacles.size(); col++)
	{
		for (int row = 0; row < Obstacles[0].size(); row++)
		{
			if (Obstacles[col][row].size() != 0 && obstacles[col][row].size() != 0 && Obstacles[col][row][0] != obstacles[col][row][0])
			{
				for (int i = 0; i < actionSpace.size(); i++)
				{
					Coordinate newCoord(col + actionSpace[i][0] * -1, row + actionSpace[i][1] * -1);
					if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
						continue;

					if (stateSpace.count(newCoord) != 0)
						UpdateVertex(static_cast<LPANode*>(stateSpace[newCoord]));
				}

				haveEdgesChanged = true;
			}
			else if ((Obstacles[col][row].size() != 0) != (obstacles[col][row].size() != 0))
			{
				for (int i = 0; i < actionSpace.size(); i++)
				{
					Coordinate newCoord(col + actionSpace[i][0] * -1, row + actionSpace[i][1] * -1);
					if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
						continue;

					if (stateSpace.count(newCoord) != 0)
						UpdateVertex(static_cast<LPANode*>(stateSpace[newCoord]));
				}

				haveEdgesChanged = true;
			}
		}
	}

	if (!firstRun && !haveEdgesChanged)
	{
		time_t endTime = time(NULL);

		exeTime += double(endTime - startTime);

		output.exeTime = exeTime;

		output.path.erase(output.path.begin());

		return output;
	}

	if (firstRun)
	{
		startNode = new LPANode(startCoord, nullptr, INTMAX_MAX, 0, start); //adds start as action from the parent just as a placeholder. Not used
		stateSpace[*startCoord] = startNode;
		queue.push_back(startNode);
	}

	firstRun = false;

	actionSpace = actions;
	obstacles = Obstacles;

	while (!queue.empty())
	{
		int minNode = 0;

		for (int i = 0; i < queue.size(); i++)
			if (std::min(queue[i]->cost, static_cast<LPANode*>(queue[i])->rhs) + Heuristic(*queue[i]->getPosition(), *goalCoord) < std::min(queue[minNode]->cost, static_cast<LPANode*>(queue[minNode])->rhs) + Heuristic(*queue[minNode]->getPosition(), *goalCoord)
			|| (abs(std::min(queue[i]->cost, static_cast<LPANode*>(queue[i])->rhs) + Heuristic(*queue[i]->getPosition(), *goalCoord) - std::min(queue[minNode]->cost, static_cast<LPANode*>(queue[minNode])->rhs) + Heuristic(*queue[minNode]->getPosition(), *goalCoord)) < 0.01
			&& std::min(queue[i]->cost, static_cast<LPANode*>(queue[i])->rhs) < std::min(queue[minNode]->cost, static_cast<LPANode*>(queue[minNode])->rhs)))
				minNode = i; 

		LPANode* curr = static_cast<LPANode*>(queue[minNode]);
		queue.erase(queue.begin() + minNode);

		if (stateSpace.count(*goalCoord) != 0)
		{
			if (std::min(curr->cost, curr->rhs) + Heuristic(*curr->getPosition(), *goalCoord) >= std::min(stateSpace[*goalCoord]->cost, static_cast<LPANode*>(stateSpace[*goalCoord])->rhs)
			|| (abs(std::min(curr->cost, curr->rhs) + Heuristic(*curr->getPosition(), *goalCoord) - std::min(stateSpace[*goalCoord]->cost, static_cast<LPANode*>(stateSpace[*goalCoord])->rhs)) < 0.01
			&& std::min(curr->cost, curr->rhs) >= std::min(stateSpace[*goalCoord]->cost, static_cast<LPANode*>(stateSpace[*goalCoord])->rhs))
			&& abs(stateSpace[*goalCoord]->cost - static_cast<LPANode*>(stateSpace[*goalCoord])->rhs) < 0.01)
				break;
		}

		if (curr->cost > curr->rhs)
			curr->cost = curr->rhs;
		else
		{
			curr->cost = INTMAX_MAX;
			UpdateVertex(curr);
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
				LPANode* child = new LPANode(newCoord, curr, INTMAX_MAX, INTMAX_MAX, actionSpace[i]);
				stateSpace[*newCoord] = child;
				queue.push_back(child);

				UpdateVertex(child);
			}
			else
			{
				LPANode* child = static_cast<LPANode*>(stateSpace[*newCoord]);

				UpdateVertex(child);
			}
		}
	}
	
	std::vector<std::vector<int>> path;

	if (stateSpace.count(*goalCoord) != 0)
	{
		Node* curr = stateSpace[*goalCoord];

		//path.push_back(curr->actionFromParent);

		while (curr->parent != nullptr && !(*curr->getPosition() == *startCoord))
		{
			path.push_back(curr->actionFromParent);
			curr = curr->parent;
		}
	}

	std::vector<std::vector<int>> outputPath;
	while (!path.empty())
	{
		outputPath.push_back(path.back());
		path.pop_back();
	}

	time_t endTime = time(NULL);

	exeTime += double(endTime - startTime);

	output = PathReturn(outputPath, nodesExpanded, exeTime);

	return output;
}

void LPA::UpdateVertex(LPANode* node)
{
	if (node != startNode)
	{
		double minRHS = INTMAX_MAX;

		for (int i = 0; i < actionSpace.size(); i++)
		{
			Coordinate newCoord(node->getPosition()->x + actionSpace[i][0] * -1, node->getPosition()->y + actionSpace[i][1] * -1);
			if (newCoord.x >= obstacles.size() || newCoord.x < 0 || newCoord.y >= obstacles[0].size() || newCoord.y < 0 || obstacles[newCoord.x][newCoord.y].size() != 0 && obstacles[newCoord.x][newCoord.y][0])
				continue;

			if (stateSpace.count(newCoord) != 0)
			{
				LPANode* parent = static_cast<LPANode*>(stateSpace[newCoord]);

				if (parent->cost + Heuristic(*parent->getPosition(), *node->getPosition()) < minRHS)
				{
					node->parent = parent;
					node->actionFromParent = actionSpace[i];
					minRHS = parent->cost + Heuristic(*parent->getPosition(), *node->getPosition());
				}
			}			
		}

		node->rhs = minRHS;
	}

	std::vector<Node*>::iterator index = std::find(queue.begin(), queue.end(), node);
	if (index != queue.end())
		queue.erase(index);

	if (abs(node->cost - node->rhs) > 0.01) //if they aren't equal
		queue.push_back(node);

	nodesExpanded++;
}

double LPA::CalcKey(LPANode* node, Coordinate* goalCoord)
{
	return std::min(std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *goalCoord), std::min(node->cost, node->rhs));
}

Node::Node(Coordinate* Position, Node* Parent, double Cost, std::vector<int> ActionFromParent)
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

LPANode::LPANode(Coordinate* Position, Node* Parent, double Cost, double RHS, std::vector<int> ActionFromParent) : Node(Position, Parent, Cost, ActionFromParent), rhs(RHS)
{
	rhs = RHS;
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
	return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
}

PathReturn::PathReturn(std::vector<std::vector<int>> Path, int NodesExpanded, double ExeTime)
{
	path = Path;
	nodesExpanded = NodesExpanded;
	exeTime = ExeTime;
}

PathReturn::PathReturn()
{
	nodesExpanded = 0;
	exeTime = 0;
}

PathFinder::PathFinder(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : obstacles(Obstacles), actionSpace(actions), nodesExpanded(0), exeTime(0), firstRun(true)
{}

double PathFinder::Heuristic(Coordinate Start, Coordinate End)
{
	return sqrt(pow(Start.x - End.x, 2) + pow(Start.y - End.y, 2));
}