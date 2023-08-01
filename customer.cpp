// customer.cpp
#include "customer.h"

Customer::Customer(float x, float y, int service_t, int tw_start, int tw_end) : Node(x, y), service_t(service_t), tw_start(tw_start), tw_end(tw_end) {}

int Customer::getStartTime()
{
    return tw_start;
}

int Customer::getEndTime()
{
    return tw_end;
}

int Customer::getServiceTime()
{
    return service_t;
}

double Customer::TimeToJ(const Node &j, float speed)
{
    int distance = this->DistanceTo(j);
    return distance / speed;
}

double Customer::Urgency(float time, const Customer &j, float speed)
{
    return j.tw_end - (time + service_t + TimeToJ(j, speed));
}

const double Customer::HeuristicValue(float time, const Customer& j, float speed, float w1)
{
    // tweak w1 between 0-1. 
    // w1 = 0 means calculation only looks for nearest nodes
    // w1 = 1 means calculation looks only for time windows
    
    float w2 = 1 - w1;

    double urgency = Urgency(time, j, speed);

    if (urgency < 0) {return - 1;} // returns -1 if its impossible to get there in time

    return 1 / (w1 * urgency + w2 * TimeToJ(j, speed));
}

// Define member functions for Customer if needed