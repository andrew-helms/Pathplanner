#include "PathFinder.h"
#include <iostream>
#include <chrono>

AStar::AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : PathFinder(actions, Obstacles)
{}

PathReturn AStar::Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal)
{
	auto startTime = std::chrono::steady_clock::now();

	Coordinate* startCoord = new Coordinate(start[0], start[1]);
	Coordinate* goalCoord = new Coordinate(goal[0], goal[1]);

	bool haveEdgesChanged = false;

	for (int col = 0; col < Obstacles.size(); col++)
	{
		for (int row = 0; row < Obstacles[0].size(); row++)
		{
			if (Obstacles[col][row].size() != 0 && obstacles[col][row].size() != 0 && Obstacles[col][row][0] != obstacles[col][row][0])
				haveEdgesChanged = true;
			else if ((Obstacles[col][row].size() == 0 && obstacles[col][row].size() != 0 && obstacles[col][row][0]) || (Obstacles[col][row].size() != 0 && Obstacles[col][row][0] && obstacles[col][row].size() == 0))
				haveEdgesChanged = true;
		}
	}

	if (!firstRun && !haveEdgesChanged)
	{
		auto endTime = std::chrono::steady_clock::now();

		exeTime += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

		output.exeTime = exeTime;

		output.path.erase(output.path.begin());

		return output;
	}

	firstRun = false;

	actionSpace = actions;
	obstacles = Obstacles;

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

	auto endTime = std::chrono::steady_clock::now();

	exeTime += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	output = PathReturn(outputPath, nodesExpanded, exeTime);

	DeconstructStateSpace();

	return output;
}

LPA::LPA(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : PathFinder(actions, Obstacles)
{}

PathReturn LPA::Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal)
{
	auto startTime = std::chrono::steady_clock::now();

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
					Coordinate newCoord(col + actionSpace[i][0] * 1, row + actionSpace[i][1] * 1);
					if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
						continue;

					if (stateSpace.count(newCoord) != 0)
						UpdateVertex(static_cast<LPANode*>(stateSpace[newCoord]), Obstacles);
				}

				haveEdgesChanged = true;
			}
			else if ((Obstacles[col][row].size() == 0 && obstacles[col][row].size() != 0 && obstacles[col][row][0]) || (Obstacles[col][row].size() != 0 && Obstacles[col][row][0] && obstacles[col][row].size() == 0))
			{
				for (int i = 0; i < actionSpace.size(); i++)
				{
					Coordinate newCoord(col + actionSpace[i][0] * 1, row + actionSpace[i][1] * 1);
					if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
						continue;

					if (stateSpace.count(newCoord) != 0)
						UpdateVertex(static_cast<LPANode*>(stateSpace[newCoord]), Obstacles);
				}

				haveEdgesChanged = true;
			}
		}
	}

	if (!firstRun && !haveEdgesChanged)
	{
		auto endTime = std::chrono::steady_clock::now();

		exeTime += std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

		output.exeTime = exeTime;

		output.path.erase(output.path.begin());

		return output;
	}

	if (firstRun)
	{
		startNode = new LPANode(startCoord, nullptr, INTMAX_MAX, 0, start); //adds start as action from the parent just as a placeholder. Not used
		stateSpace[*startCoord] = startNode;
		queue.push_back(startNode);

		nodesExpanded++;
	}

	startNode = static_cast<LPANode*>(stateSpace[*startCoord]);
	startNode->cost = INTMAX_MAX;
	startNode->rhs = 0;
	UpdateVertex(startNode, Obstacles);

	firstRun = false;

	actionSpace = actions;
	obstacles = Obstacles;

	while (!queue.empty())
	{
		int minNode = 0;

		for (int i = 0; i < queue.size(); i++)
			if (CalcKey(static_cast<LPANode*>(queue[i]), goalCoord, static_cast<LPANode*>(queue[minNode])))
				minNode = i; 

		LPANode* curr = static_cast<LPANode*>(queue[minNode]);
		queue.erase(queue.begin() + minNode);

		if (stateSpace.count(*goalCoord) != 0)
		{
			if (!(CalcKey(curr, goalCoord, static_cast<LPANode*>(stateSpace[*goalCoord])) || fabs(stateSpace[*goalCoord]->cost - static_cast<LPANode*>(stateSpace[*goalCoord])->rhs) > 0.000001))
			{
				double grhs = static_cast<LPANode*>(stateSpace[*goalCoord])->rhs;
				double diff = fabs(stateSpace[*goalCoord]->cost - static_cast<LPANode*>(stateSpace[*goalCoord])->rhs);
				double currh = Heuristic(*curr->getPosition(), *goalCoord);
				break;
			}
		}

		if (curr->cost > curr->rhs)
		{
			curr->cost = curr->rhs;

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

					nodesExpanded++;

					UpdateVertex(child, obstacles);
				}
				else
				{
					LPANode* child = static_cast<LPANode*>(stateSpace[*newCoord]);

					UpdateVertex(child, obstacles);

					delete newCoord;
				}
			}
		}
		else
		{
			curr->cost = INTMAX_MAX;

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

					UpdateVertex(child, obstacles);
				}
				else
				{
					LPANode* child = static_cast<LPANode*>(stateSpace[*newCoord]);

					UpdateVertex(child, obstacles);

					delete newCoord;
				}
			}

			UpdateVertex(curr, obstacles);
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

	auto endTime = std::chrono::steady_clock::now();

	exeTime += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	output = PathReturn(outputPath, nodesExpanded, exeTime);

	for (int i = 0; i < outputPath.size(); i++)
	{
		//std::cout << outputPath[i][0] << " " << outputPath[i][1] << std::endl;
	}

	//std::cout << std::endl;

	return output;
}

void LPA::UpdateVertex(LPANode* node, std::vector<std::vector<std::vector<bool>>>& Obstacles)
{
	if (node != startNode)
	{
		double minRHS = INTMAX_MAX;
		node->parent = nullptr;

		for (int i = 0; i < actionSpace.size(); i++)
		{
			Coordinate newCoord(node->getPosition()->x + actionSpace[i][0] * -1, node->getPosition()->y + actionSpace[i][1] * -1);
			if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
				continue;

			if (stateSpace.count(newCoord) != 0)
			{
				LPANode* parent = static_cast<LPANode*>(stateSpace[newCoord]);

				if (parent->cost + Heuristic(*parent->getPosition(), *node->getPosition()) < minRHS)
				{
					if (!(parent->parent == node))
					{
						node->parent = parent;
						node->actionFromParent = actionSpace[i];
						minRHS = parent->cost + Heuristic(*parent->getPosition(), *node->getPosition());
					}
				}
			}			
		}

		node->rhs = minRHS;
	}

	std::vector<Node*>::iterator index = std::find(queue.begin(), queue.end(), node);
	if (index != queue.end())
		queue.erase(index);

	if (fabs(node->cost - node->rhs) > 0.000001) //if they aren't equal
		queue.push_back(node);
}

bool LPA::CalcKey(LPANode* node, Coordinate* goalCoord, LPANode* rhs) //true if key(node) < key(rhs)
{
	bool aStarKey = (std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *goalCoord) < std::min(rhs->cost, rhs->rhs) + Heuristic(*rhs->getPosition(), *goalCoord));
	bool tie = fabs(std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *goalCoord) - std::min(rhs->cost, rhs->rhs) + Heuristic(*rhs->getPosition(), *goalCoord)) < 0.00001;
	bool dijkstraKey = (std::min(node->cost, node->rhs) < std::min(rhs->cost, rhs->rhs));
	return aStarKey || (tie && dijkstraKey);
}

DStar::DStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : PathFinder(actions, Obstacles)
{}

PathReturn DStar::Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal)
{
	auto startTime = std::chrono::steady_clock::now();

	Coordinate* startCoord = new Coordinate(start[0], start[1]);
	Coordinate* goalCoord = new Coordinate(goal[0], goal[1]);

	bool haveEdgesChanged = false;

	for (int col = 0; col < Obstacles.size(); col++)
	{
		for (int row = 0; row < Obstacles[0].size(); row++)
		{
			if (Obstacles[col][row].size() != 0 && obstacles[col][row].size() != 0 && Obstacles[col][row][0] != obstacles[col][row][0])
			{
				Coordinate currPos(col, row);

				if (stateSpace.count(currPos) != 0)
				{
					for (int i = 0; i < actionSpace.size(); i++)
					{
						Coordinate newCoord(col + actionSpace[i][0] * -1, row + actionSpace[i][1] * -1);
						if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
							continue;

						if (stateSpace.count(newCoord) != 0)
						{
							if (obstacles[col][row][0])
								if (!(newCoord == *goalCoord))
								static_cast<DStarNode*>(stateSpace[newCoord])->rhs = std::min(static_cast<DStarNode*>(stateSpace[newCoord])->rhs, Heuristic(currPos, newCoord) + stateSpace[currPos]->cost);
							else if (/*fabs(static_cast<DStarNode*>(stateSpace[newCoord])->rhs - (1 + stateSpace[currPos]->cost)) < 0.00001 && */!(newCoord == *goalCoord))
							{
								double minCost = INTMAX_MAX;

								for (int j = 0; j < actionSpace.size(); j++)
								{
									Coordinate childCoord(col + actionSpace[j][0], row + actionSpace[j][1]);
									if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
										continue;

									if (stateSpace.count(childCoord) != 0)
									{
										minCost = std::min(minCost, Heuristic(childCoord, newCoord) + stateSpace[childCoord]->cost);
									}
								}

								static_cast<DStarNode*>(stateSpace[newCoord])->rhs = minCost;
							}

							UpdateVertex(static_cast<DStarNode*>(stateSpace[newCoord]), Obstacles, startCoord);
						}
					}
				}

				haveEdgesChanged = true;
			}
			else if ((Obstacles[col][row].size() == 0 && obstacles[col][row].size() != 0 && obstacles[col][row][0]) || (Obstacles[col][row].size() != 0 && Obstacles[col][row][0] && obstacles[col][row].size() == 0))
			{
				Coordinate currPos(col, row);

				if (stateSpace.count(currPos) != 0)
				{
					for (int i = 0; i < actionSpace.size(); i++)
					{
						Coordinate newCoord(col + actionSpace[i][0] * -1, row + actionSpace[i][1] * -1);
						if (newCoord.x >= Obstacles.size() || newCoord.x < 0 || newCoord.y >= Obstacles[0].size() || newCoord.y < 0 || Obstacles[newCoord.x][newCoord.y].size() != 0 && Obstacles[newCoord.x][newCoord.y][0])
							continue;

						if (stateSpace.count(newCoord) != 0)
						{
							if (obstacles[col][row].size() > 0 && obstacles[col][row][0])
								if (!(newCoord == *goalCoord))
									static_cast<DStarNode*>(stateSpace[newCoord])->rhs = std::min(static_cast<DStarNode*>(stateSpace[newCoord])->rhs, Heuristic(newCoord, currPos) + stateSpace[currPos]->cost);
							else if (/*fabs(static_cast<DStarNode*>(stateSpace[newCoord])->rhs - (1 + stateSpace[currPos]->cost)) < 0.00001 && */!(newCoord == *goalCoord))
							{
								double minCost = INTMAX_MAX;

								for (int j = 0; j < actionSpace.size(); j++)
								{
									Coordinate childCoord(col + actionSpace[j][0], row + actionSpace[j][1]);
									if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
										continue;

									if (stateSpace.count(childCoord) != 0)
									{
										minCost = std::min(minCost, Heuristic(childCoord, newCoord) + stateSpace[childCoord]->cost);
									}
								}

								static_cast<DStarNode*>(stateSpace[newCoord])->rhs = minCost;
							}

							UpdateVertex(static_cast<DStarNode*>(stateSpace[newCoord]), Obstacles, startCoord);
						}
					}
				}

				haveEdgesChanged = true;
			}
		}
	}

	if (!firstRun && !haveEdgesChanged)
	{
		auto endTime = std::chrono::steady_clock::now();

		exeTime += std::chrono::duration_cast<std::chrono::seconds>(endTime - startTime).count();

		output.exeTime = exeTime;

		output.path.erase(output.path.begin());

		return output;
	}

	if (firstRun)
	{
		stateSpace[*goalCoord] = new DStarNode(goalCoord, nullptr, INTMAX_MAX, 0, Heuristic(*startCoord, *goalCoord), 0, start); //adds start as action from the parent just as a placeholder. Not used;
		queue.push_back(static_cast<DStarNode*>(stateSpace[*goalCoord]));

		nodesExpanded++;
	}

	firstRun = false;

	actionSpace = actions;
	obstacles = Obstacles;

	int Counter = 0;

	while (!queue.empty())
	{
		/*std::cout << "We are stuck in queue.empty() " << Counter << std::endl;
		Counter++;*/
		int minNode = 0;
		for (int i = 0; i < queue.size(); i++)
			if ((queue[i]->KeyTop < queue[minNode]->KeyTop) || (fabs(queue[i]->KeyTop - queue[minNode]->KeyTop) < 0.00001 && (queue[i]->KeyBottom < queue[minNode]->KeyBottom)))
				minNode = i;

		double KoldTop = queue[minNode]->KeyTop;
		double koldBottom = queue[minNode]->KeyBottom;
		DStarNode* curr = queue[minNode];
		queue.erase(queue.begin() + minNode);

		if (stateSpace.count(*startCoord) != 0)
		{
			if ((curr->KeyTop > static_cast<DStarNode*>(stateSpace[*startCoord])->KeyTop || fabs(curr->KeyTop - static_cast<DStarNode*>(stateSpace[*startCoord])->KeyTop) < 0.00001) && (curr->KeyBottom > static_cast<DStarNode*>(stateSpace[*startCoord])->KeyBottom))
			{
				if (static_cast<DStarNode*>(stateSpace[*startCoord])->rhs == static_cast<DStarNode*>(stateSpace[*startCoord])->cost)
				{
					break;
				}
			}
		}

		//First we check if KeyTop and KeyBottom have changed since we got this node.
		double NewCurrKeyTop = std::min(curr->cost, curr->rhs) + Heuristic(*curr->getPosition(), *startCoord) + Km;
		double NewCurrKeyBottom = std::min(curr->cost, curr->rhs);
		if ((curr->KeyTop < NewCurrKeyTop) || (fabs(curr->KeyTop - NewCurrKeyTop) < 0.00001) && (curr->KeyBottom < NewCurrKeyBottom))
		{
			curr->KeyTop = NewCurrKeyTop;
			curr->KeyBottom = NewCurrKeyBottom;
			queue.push_back(curr);
		}
		else if (curr->cost > curr->rhs)
		{
			curr->cost = curr->rhs;

			//This makes sure we erase curr, if we haven't already.
			std::vector<DStarNode*>::iterator index = std::find(queue.begin(), queue.end(), curr);
			if (index != queue.end())
				queue.erase(index);

			for (int i = 0; i < actionSpace.size(); i++)
			{
				//Get the inverse action
				std::vector<int> invAct;
				invAct.push_back(actionSpace[i][0] * -1);
				invAct.push_back(actionSpace[i][1] * -1);

				Coordinate* newCoord = new Coordinate(curr->getPosition()->x + invAct[0], curr->getPosition()->y + invAct[1]);
				//Make a new coordinate for each of the predecessors, skip any obstacles.
				if (newCoord->x >= Obstacles.size() || newCoord->x < 0 || newCoord->y >= Obstacles[0].size() || newCoord->y < 0 || Obstacles[newCoord->x][newCoord->y].size() != 0 && Obstacles[newCoord->x][newCoord->y][0])
					continue;

				//If we have made a node for this element before
				if (stateSpace.count(*newCoord) != 0)
				{
					//Get said node
					DStarNode* parent = static_cast<DStarNode*>(stateSpace[*newCoord]);

					//If it isn't the goal coordinate, then we set its rhs value equal to the minimum of all of its successors,
					//if we run into a successor we haven't visited yet, then we ignore it since then it's equal to infinity
					//so it clearly won't be a minimum as we have at least visited the node we used to get to this parent.
					double minRhs = INTMAX_MAX;
					if (!(*newCoord == *goalCoord))
					{
						for (int k = 0; k < actionSpace.size(); k++)
						{
							Coordinate* CurrentSuccessor = new Coordinate(parent->getPosition()->x + actionSpace[k][0], parent->getPosition()->y + actionSpace[k][1]);
							if (CurrentSuccessor->x >= Obstacles.size() || CurrentSuccessor->x < 0 || CurrentSuccessor->y >= Obstacles[0].size() || CurrentSuccessor->y < 0 || Obstacles[CurrentSuccessor->x][CurrentSuccessor->y].size() != 0 && Obstacles[CurrentSuccessor->x][CurrentSuccessor->y][0])
								continue;
							if (stateSpace.count(*CurrentSuccessor) != 0)
							{
								double newRhs = Heuristic(*static_cast<DStarNode*>(stateSpace[*CurrentSuccessor])->getPosition(), *parent->getPosition()) + static_cast<DStarNode*>(stateSpace[*CurrentSuccessor])->cost;
								if (minRhs > newRhs)
								{
									parent->parent = static_cast<DStarNode*>(stateSpace[*CurrentSuccessor]);
									minRhs = newRhs;
								}
							}
						}
						parent->rhs = minRhs;
					}
						//parent->rhs = std::min(parent->rhs, Heuristic(*newCoord, *curr->getPosition()) + curr->cost);

					UpdateVertex(parent, obstacles, startCoord);

					delete newCoord;
				}
				else
				{
					DStarNode* parent = new DStarNode(newCoord, curr, INTMAX_MAX, INTMAX_MAX, INTMAX_MAX, INTMAX_MAX, invAct);
					stateSpace[*newCoord] = parent;

					nodesExpanded++;

					double minRhs = INTMAX_MAX;
					if (!(*newCoord == *goalCoord))
					{
						for (int k = 0; k < actionSpace.size(); k++)
						{
							Coordinate* CurrentSuccessor = new Coordinate(parent->getPosition()->x + actionSpace[k][0], parent->getPosition()->y + actionSpace[k][1]);
							if (CurrentSuccessor->x >= Obstacles.size() || CurrentSuccessor->x < 0 || CurrentSuccessor->y >= Obstacles[0].size() || CurrentSuccessor->y < 0 || Obstacles[CurrentSuccessor->x][CurrentSuccessor->y].size() != 0 && Obstacles[CurrentSuccessor->x][CurrentSuccessor->y][0])
								continue;
							if (stateSpace.count(*CurrentSuccessor) != 0)
							{
								double newRhs = Heuristic(*static_cast<DStarNode*>(stateSpace[*CurrentSuccessor])->getPosition(), *parent->getPosition()) + static_cast<DStarNode*>(stateSpace[*CurrentSuccessor])->cost;
								if (minRhs > newRhs)
								{
									parent->parent = static_cast<DStarNode*>(stateSpace[*CurrentSuccessor]);
									minRhs = newRhs;
								}
							}
						}
						parent->rhs = minRhs;
					}
					UpdateVertex(parent, obstacles, startCoord);
				}
			}
		}
		else
		{
			double oldCost = curr->cost;
			curr->cost = INTMAX_MAX;

			if (!(*curr->getPosition() == *goalCoord))
			{
			}

			if (curr->rhs == oldCost && !(*curr->getPosition() == *goalCoord))
			{
				double minCost = INTMAX_MAX;
				//DStarNode* parent = nullptr;

				for (int j = 0; j < actionSpace.size(); j++)
				{
					Coordinate childCoord = Coordinate(curr->getPosition()->x + actionSpace[j][0], curr->getPosition()->y + actionSpace[j][1]);
					if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
						continue;

					if (stateSpace.count(childCoord) != 0)
					{
						Node* child = stateSpace[childCoord];
						double childCost = Heuristic(*curr->getPosition(), *child->getPosition()) + child->cost;

						if (childCost < minCost)
						{
							curr->parent = child;
							minCost = childCost;
							//parent = static_cast<DStarNode*>(child);
						}
					}
				}
				
				curr->rhs = minCost;
				//curr->parent = parent;
			}

			UpdateVertex(curr, obstacles, startCoord);

			for (int i = 0; i < actionSpace.size(); i++)
			{
				std::vector<int> invAct;
				invAct.push_back(actionSpace[i][0] * -1);
				invAct.push_back(actionSpace[i][1] * -1);

				Coordinate* newCoord = new Coordinate(curr->getPosition()->x + invAct[0], curr->getPosition()->y + invAct[1]);
				if (newCoord->x >= Obstacles.size() || newCoord->x < 0 || newCoord->y >= Obstacles[0].size() || newCoord->y < 0 || Obstacles[newCoord->x][newCoord->y].size() != 0 && Obstacles[newCoord->x][newCoord->y][0])
					continue;

				if (stateSpace.count(*newCoord) != 0)
				{
					DStarNode* parent = static_cast<DStarNode*>(stateSpace[*newCoord]);

					if (!(*newCoord == *goalCoord))/* && parent->rhs == (Heuristic(*newCoord, *curr->getPosition()) + oldCost))*/
					{
						double minCost = INTMAX_MAX;
						//DStarNode* parent = nullptr;

						for (int j = 0; j < actionSpace.size(); j++)
						{
							Coordinate childCoord = Coordinate(parent->getPosition()->x + actionSpace[j][0], parent->getPosition()->y + actionSpace[j][1]);
							if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
								continue;

							if (stateSpace.count(childCoord) != 0)
							{
								Node* child = stateSpace[childCoord];
								double childCost = Heuristic(*parent->getPosition(), *child->getPosition()) + child->cost;

								if (childCost < minCost)
								{
									minCost = childCost;
									parent->parent = child;
									//parent = static_cast<DStarNode*>(child);
								}
							}
						}

						parent->rhs = minCost;
						//curr->parent = parent;
					}

					UpdateVertex(parent, obstacles, startCoord);

					delete newCoord;
				}
				else
				{
					DStarNode* parent = new DStarNode(newCoord, curr, INTMAX_MAX, INTMAX_MAX, INTMAX_MAX, INTMAX_MAX, invAct);
					stateSpace[*newCoord] = parent;

						if (!(*newCoord == *goalCoord))/* && parent->rhs == (Heuristic(*newCoord, *curr->getPosition()) + oldCost))*/
						{
							double minCost = INTMAX_MAX;
							//DStarNode* parent = nullptr;
	
							for (int j = 0; j < actionSpace.size(); j++)
							{
								Coordinate childCoord = Coordinate(parent->getPosition()->x + actionSpace[j][0], parent->getPosition()->y + actionSpace[j][1]);
								if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
									continue;
									
								if (stateSpace.count(childCoord) != 0)
								{
									Node* child = stateSpace[childCoord];
									double childCost = Heuristic(*curr->getPosition(), *child->getPosition()) + child->cost;
										
									if (childCost < minCost)
									{
										parent->parent = child;
										minCost = childCost;
										//parent = static_cast<DStarNode*>(child);
									}
								}
							}

							parent->rhs = minCost;
							//curr->parent = parent;
						}

					nodesExpanded++;

					UpdateVertex(parent, obstacles, startCoord);
				}
			}
		}
	}

	DStarNode* curr = static_cast<DStarNode*>(stateSpace[*startCoord]);
	std::vector<int> StartData;
	std::vector<std::vector<int>> path;
	std::vector<std::vector<int>> TempRoute;
	int TestCount = 0;
	std::vector<int> Initial;
	Initial.push_back(curr->getPosition()->x);
	Initial.push_back(curr->getPosition()->y);
	TempRoute.push_back(Initial);
	while (!(*curr->getPosition() == *goalCoord))
	{

		//std::cout << "Current position: " << curr->getPosition()->x << " " << curr->getPosition()->y << std::endl;

		double minCost = INTMAX_MAX;
		std::vector<int> bestAction;
		DStarNode* child = nullptr;
		//std::cout << "Searching for next action" << std::endl;
		for (int j = 0; j < actionSpace.size(); j++)
		{
			Coordinate childCoord = Coordinate(curr->getPosition()->x + actionSpace[j][0], curr->getPosition()->y + actionSpace[j][1]);
			if (childCoord.x >= Obstacles.size() || childCoord.y >= Obstacles[0].size())
				continue;
			if (Obstacles[childCoord.x][childCoord.y].size() > 0)
			{
				if (Obstacles[childCoord.x][childCoord.y][0])
				{
					continue;
				}
			}
			bool AlreadyPassed = false;
			int RouteParser = 0;
			//std::cout << "Printing route: ";
			while (RouteParser < Route.size())
			{
				//std::cout << " n " << Route[RouteParser][0] << " " << Route[RouteParser][1];
				if((childCoord.x == Route[RouteParser][0])&&(childCoord.y == Route[RouteParser][1]))
				{
					AlreadyPassed = true;
				}
				RouteParser++;
			}
			//std::cout << std::endl;
			if(AlreadyPassed)
			{
				continue;
			}
			bool AlreadyAdded = false;
			RouteParser = 0;
			//std::cout << "Printing temp route: ";
			while (RouteParser < TempRoute.size())
			{
				//std::cout << " n " << TempRoute[RouteParser][0] << " " << TempRoute[RouteParser][1];
				if ((childCoord.x == TempRoute[RouteParser][0]) && (childCoord.y == TempRoute[RouteParser][1]))
				{
					AlreadyAdded = true;
				}
				RouteParser++;
			}
			if(AlreadyAdded)
			{
				continue;
			}
			//std::cout << std::endl;
			/*if (childCoord.x >= Obstacles.size() || childCoord.x < 0 || childCoord.y >= Obstacles[0].size() || childCoord.y < 0 || Obstacles[childCoord.x][childCoord.y].size() != 0 && Obstacles[childCoord.x][childCoord.y][0])
				continue;*/
			//It originally was Heuristic(*curr->getPosition(), childCoord)
			//std::cout << "Current coord: " << childCoord.x << " " << childCoord.y << std::endl;
			//std::cout << "Coord value: " << Heuristic(*curr->getPosition(), childCoord) + static_cast<DStarNode*>(stateSpace[childCoord])->cost << std::endl;
			//std::cout << "StateSpace.count: " << stateSpace.count(childCoord) << std::endl;
			if (stateSpace.count(childCoord) != 0 && Heuristic(*curr->getPosition(), childCoord) + static_cast<DStarNode*>(stateSpace[childCoord])->cost < minCost)
			{
				minCost = Heuristic(*curr->getPosition(), childCoord) + stateSpace[childCoord]->cost;
				bestAction = actionSpace[j];
				child = static_cast<DStarNode*>(stateSpace[childCoord]);
			}
		}

		//std::cout << bestAction.size() << " BestAction's size" << std::endl;
		if (bestAction.size() == 0)
		{
			std::cout << curr->getPosition()->x << " " << curr->getPosition()->y << std::endl;
			for (int j = 0; j < actionSpace.size(); j++)
			{
				Coordinate childCoord = Coordinate(curr->getPosition()->x + actionSpace[j][0], curr->getPosition()->y + actionSpace[j][1]);
				//std::cout << "Child Pos: " << childCoord.x << " " << childCoord.y << std::endl;
				if (childCoord.x >= Obstacles.size() || childCoord.y >= Obstacles[0].size())
				{
					//std::cout << "It's an obstacle" << std::endl;
					continue;
				}
				if (Obstacles[childCoord.x][childCoord.y].size() > 0)
				{
					if (Obstacles[childCoord.x][childCoord.y][0])
					{
					//std::cout << "It's an obstacle" << std::endl;
						continue;
					}
				}
				if (stateSpace.count(childCoord) == 0)
				{
					//std::cout << "We never added this node to the state space" << std::endl;
					continue;
				}
				bool AlreadyPassed = false;
				int RouteParser = 0;
				//std::cout << "Printing route: ";
				while (RouteParser < Route.size())
				{
					//std::cout << " n " << Route[RouteParser][0] << " " << Route[RouteParser][1];
					if ((childCoord.x == Route[RouteParser][0]) && (childCoord.y == Route[RouteParser][1]))
					{
						AlreadyPassed = true;
					}
					RouteParser++;
				}
				//std::cout << std::endl;
				if (AlreadyPassed)
				{
					//std::cout << "We already added this node to our route" << std::endl;
					continue;
				}
				bool AlreadyAdded = false;
				RouteParser = 0;
				//std::cout << "Printing temp route: ";
				while (RouteParser < TempRoute.size())
				{
					//std::cout << " n " << TempRoute[RouteParser][0] << " " << TempRoute[RouteParser][1];
					if ((childCoord.x == TempRoute[RouteParser][0]) && (childCoord.y == TempRoute[RouteParser][1]))
					{
						AlreadyAdded = true;
					}
					RouteParser++;
				}
				//std::cout << std::endl;
				if (AlreadyAdded)
				{
					//std::cout << "We already added this node to our temp route" << std::endl;
					continue;
				}
				//std::cout << "Cost: " << Heuristic(*curr->getPosition(), childCoord) + static_cast<DStarNode*>(stateSpace[childCoord])->cost << std::endl;

			}
		}
		std::vector<int> NewPosition;
		NewPosition.push_back(curr->getPosition()->x + bestAction[0]);
		NewPosition.push_back(curr->getPosition()->y + bestAction[1]);
		TempRoute.push_back(NewPosition);
		path.push_back(bestAction);
		if (child != nullptr)
			curr = child;
		else
			break;
	}
	
	if (path.size() > 0)
	{
		if (Route.size() == 0)
		{
			Coordinate NewStart = Coordinate(startCoord->x + path[0][0], startCoord->y + path[0][1]);
			std::vector<int> PathedPosition;
			PathedPosition.push_back(NewStart.x);
			PathedPosition.push_back(NewStart.y);
			//std::cout << "Adding " << PathedPosition[0] << " " << PathedPosition[1] << " to the Route ";
			//std::cout << "Path was " << path[0][0] << " " << path[0][1] << std::endl;
			Route.push_back(PathedPosition);
			Km = Km + Heuristic(*startCoord, NewStart);
		}
		else
		{
			Coordinate NewStart = Coordinate(Route[Route.size() - 1][0] + path[0][0], Route[Route.size() - 1][1] + path[0][1]);
			std::vector<int> PathedPosition;
			PathedPosition.push_back(NewStart.x);
			PathedPosition.push_back(NewStart.y);
			//std::cout << "Adding " << PathedPosition[0] << " " << PathedPosition[1] << " to the Route ";
			//std::cout << "Path was " << path[0][0] << " " << path[0][1] << std::endl;
			Route.push_back(PathedPosition);
			Km = Km + Heuristic(*startCoord, NewStart);
		}
	}

	auto endTime = std::chrono::steady_clock::now();

	exeTime += std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	output = PathReturn(path, nodesExpanded, exeTime);

	return output;
}

void DStar::UpdateVertex(DStarNode* node, std::vector<std::vector<std::vector<bool>>>& Obstacles, Coordinate* startCoord)
{
	std::vector<DStarNode*>::iterator index = std::find(queue.begin(), queue.end(), node);
	std::vector<DStarNode*>::iterator end = queue.end();
		
	if ((index == end) && (fabs((double)(node->cost - node->rhs)) > 0.0001))
	{
		//Now before we add it we need to recalculate the key.
		node->KeyTop = std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *startCoord) + Km;
		node->KeyBottom = std::min(node->cost, node->rhs);
		queue.push_back(node);
	}
	
	if (!(index == end) && (fabs((double)(node->cost - node->rhs)) < 0.0001))
	{
		queue.erase(index);
	}
}

bool DStar::CalcKey(DStarNode* node, Coordinate* startCoord, DStarNode* rhs)
{
	bool aStarKey = (std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *startCoord) < std::min(rhs->cost, rhs->rhs) + Heuristic(*rhs->getPosition(), *startCoord));
	bool tie = fabs(std::min(node->cost, node->rhs) + Heuristic(*node->getPosition(), *startCoord) - std::min(rhs->cost, rhs->rhs) + Heuristic(*rhs->getPosition(), *startCoord)) < 0.00001;
	bool dijkstraKey = (std::min(node->cost, node->rhs) < std::min(rhs->cost, rhs->rhs));
	return aStarKey || (tie && dijkstraKey);
}

Node::Node(Coordinate* Position, Node* Parent, double Cost, std::vector<int> ActionFromParent) : position(Position), parent(Parent), cost(Cost), actionFromParent(ActionFromParent)
{}

Coordinate* Node::getPosition()
{
	return position;
}

LPANode::LPANode(Coordinate* Position, Node* Parent, double Cost, double RHS, std::vector<int> ActionFromParent) : Node(Position, Parent, Cost, ActionFromParent), rhs(RHS)
{}

DStarNode::DStarNode(Coordinate* Position, Node* Parent, double Cost, double RHS, double KeyTop, double KeyBottom, std::vector<int> ActionFromParent) : Node(Position, Parent, Cost, ActionFromParent), rhs(RHS)
{}

Coordinate::Coordinate(int X, int Y) : x(X), y(Y)
{}

bool operator==(const Coordinate& lhs, const Coordinate& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y;
}

bool operator<(const Coordinate& lhs, const Coordinate& rhs)
{
	return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
}

PathReturn::PathReturn(std::vector<std::vector<int>> Path, int NodesExpanded, double ExeTime) : path(Path), nodesExpanded(NodesExpanded), exeTime(ExeTime)
{}

PathReturn::PathReturn() : nodesExpanded(0), exeTime(0)
{}

PathFinder::PathFinder(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles) : obstacles(Obstacles), actionSpace(actions), nodesExpanded(0), exeTime(0), firstRun(true)
{}

double PathFinder::Heuristic(Coordinate Start, Coordinate End)
{
	return sqrt(pow(Start.x - End.x, 2) + pow(Start.y - End.y, 2));
}

void PathFinder::DeconstructStateSpace()
{
	for (std::map<Coordinate, Node*>::iterator it = stateSpace.begin(); it != stateSpace.end(); it++)
	{
		Node* temp = it->second;
		delete temp;
	}

	stateSpace.clear();
}