#include "AStar.h"

AStar::AStar(std::vector<std::vector<int>> actions, std::vector<std::vector<std::vector<bool>>> states)
{
	actionSpace = actions;
}

std::vector<std::vector<int>> AStar::findPath(std::vector<int> start, std::vector<int> goal)
{
	return actionSpace;
}

Node::Node(int x, int y, Node* Parent)
{
	position = new Coordinate(x,y);

	parent = Parent;
}

std::vector<int> Node::getActionFromParent()
{
	return actionFromParent;
}

Node* Node::getParent()
{
	return parent;
}

Coordinate Node::getPosition()
{
	return *position;
}

Coordinate::Coordinate(int X, int Y)
{
	x = X;
	y = Y;
}

bool Coordinate::operator==(const Coordinate& rhs)
{
	return this->x == rhs.x && this->y == rhs.y;
}