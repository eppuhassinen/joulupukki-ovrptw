// node.h
#ifndef NODE_H
#define NODE_H

class Node
{
public:
    Node(float x, float y); // Constructor

    float getX() const;       // Getter for x coordinate
    float getY() const;       // Getter for y coordinate
    float DistanceTo(const Node& j); // distance to node j

    

protected:
    float x, y; // Coordinates x and y for Node
};

#endif // NODE_H