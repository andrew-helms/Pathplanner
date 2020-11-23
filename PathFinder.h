#pragma once
#include <vector>
#include <map>
#include "time.h"
#include <algorithm>

class Coordinate {
public:
	int x;
	int y;

	Coordinate(int X, int Y);
};

bool operator <(const Coordinate& lhs, const Coordinate& rhs);
bool operator==(const Coordinate& lhs, const Coordinate& rhs);

class Node {
public:
	Node(Coordinate* Position, Node* Parent, int Cost, std::vector<int> ActionFromParent);
	Coordinate* getPosition();

	int cost;
	std::vector<int> actionFromParent;
	Node* parent;

protected:
	Coordinate* position;
};

class PathReturn {
public:
	PathReturn(std::vector<std::vector<int>> Path, int NodesExpanded, double ExeTime);

	std::vector<std::vector<int>> path;
	int nodesExpanded;
	double exeTime;
};

class PathFinder {
public:
	PathFinder(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles);
	virtual PathReturn* Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal) = 0;

protected:
	std::vector<std::vector<int>> actionSpace;
	std::vector<std::vector<std::vector<bool>>> obstacles;
	std::map<Coordinate, Node*> stateSpace;
	int nodesExpanded;
	double exeTime;

	int Heuristic(Coordinate Start, Coordinate End);
};

class AStar : public PathFinder {
public:
	AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles);
	PathReturn* Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal);
};

class LPA : public PathFinder {
public:
	LPA(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles);
	PathReturn* Update(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles, std::vector<int> start, std::vector<int> goal);
};

class LPANode : public Node {
public:
	LPANode(Coordinate* Position, Node* Parent, int Cost, int RHS, std::vector<int> ActionFromParent);

	int rhs;
};