#pragma once
#include <vector>
#include <map>

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

private:
	Coordinate* position;
};

class AStar {
public:
	AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> Obstacles);
	std::vector<std::vector<int>> findPath(std::vector<int> start, std::vector<int> goal);

private:
	std::vector<std::vector<int>> actionSpace;
	std::vector<std::vector<std::vector<bool>>> obstacles;
	std::map<Coordinate, Node*> stateSpace;
	int Heuristic(Coordinate Start, Coordinate End);
};