// customer.h
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "node.h"

class Customer : public Node
{
public:
    Customer(float x, float y, int service_t, int tw_start, int tw_end); // Constructor

    int getStartTime();

    int getEndTime();

    int getServiceTime();

    // travel time to j
    double TimeToJ(const Node &j, float speed); 

    // calculates the urgency factor to travel to node j
    double Urgency(float time, const Customer& j, float speed); 

    // calculates the heuristic value to travel to the node j according to the time window and distance
    const double HeuristicValue(float time, const Customer& j, float speed, float w1);

private:
    int service_t = 20; // service_time is the time spent with customer
    int tw_start = 0;   // minutes from start time
    int tw_end = 900;   // 900 minutes is 15 h
};

#endif // CUSTOMER_H