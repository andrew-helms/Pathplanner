#pragma once
#include <vector>
#include <map>

class Coordinate {
public:
	int x;
	int y;

	Coordinate(int X, int Y);
	bool operator ==(const Coordinate& rhs);
};

class Node {
public:
	Node(int x, int y, Node* parent);
	std::vector<int> getActionFromParent();
	Node* getParent();
	Coordinate getPosition();

private:
	Coordinate* position;
	Node* parent;
	std::vector<int> actionFromParent;
};

class AStar {
public:
	AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> states);
	std::vector<std::vector<int>> findPath(std::vector<int> start, std::vector<int> goal);

private:
	std::vector<std::vector<int>> actionSpace;
	std::map<Coordinate, Node> stateSpace;
};