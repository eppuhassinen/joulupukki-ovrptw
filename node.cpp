// node.cpp
#include "node.h"

#include <cmath>

Node::Node(float x, float y) : x(x), y(y) {}


float Node::getX() const {
    return x;
}

float Node::getY() const {
    return y;
}

float Node::DistanceTo(const Node& j)
{
    float xDistance = j.getX() - this->getX();
    float yDistance = j.getY() - this->getY();
    return std::sqrt(std::pow(xDistance, 2) + std::pow(yDistance, 2));
}



// Define member functions for Node if needed