#include "customer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <random>
#include <chrono>

using namespace std;

// Saves paths to a CSV file routes.csv
void savePathsToCSV(const std::vector<std::vector<int>> &paths)
{
    std::ofstream outputFile("routes.csv");
    if (!outputFile.is_open())
    {
        std::cout << "Error opening file 'routes.csv'" << std::endl;
        return;
    }

    // Iterate through each path in the outer vector
    for (const auto &path : paths)
    {
        // Write the elements of the inner vector separated by commas
        for (size_t i = 0; i < path.size(); ++i)
        {
            outputFile << path[i];
            // Add a comma if not the last element in the inner vector
            if (i < path.size() - 1)
                outputFile << ";";
        }
        // Start a new line for the next path
        outputFile << "\n";
    }

    outputFile.close();
    std::cout << "Paths saved to 'routes.csv'" << std::endl;
}

// Reads customers or depots from a CSV file.
// service time is minutes. For depots it's 0
std::vector<Customer> readDataFromCSV(const std::string &filename, int service_time = 0)
{
    std::vector<Customer> customers;

    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return customers;
    }

    std::string line;
    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        float x, y;
        int tw_start, tw_end;

        // Read the CSV fields and store them in variables
        char delimiter = ';';
        if (!(ss >> x >> delimiter >> y >> delimiter >> tw_start >> delimiter >> tw_end))
        {
            std::cerr << "Error parsing line: " << line << std::endl;
            continue; // Skip this line and move to the next
        }

        // Create a Customer object and add it to the vector
        customers.emplace_back(x, y, service_time, tw_start, tw_end);
    }

    file.close();

    return customers;
}

// creates a two dimensional vector containing the id values of the Nodes. Last columns of the two-dimensional vector are the depot ids.
vector<vector<float>> CreatePheromoneMap(int n_customers, int n_depots, float init_val = 1)
{
    vector<vector<float>> pheromone_map(n_customers + n_depots);

    vector<float> values(n_customers + n_depots);

    fill(values.begin(), values.end(), init_val);

    fill(pheromone_map.begin(), pheromone_map.end(), values);

    return pheromone_map;
}

/// @brief Calculates the next node
/// @param i current customer
/// @param customers all the customers in a vector
/// @param pheromoneMap vector<float> is containing the edge pheromone values from i to customers in pheromoneMap
/// @param r0 0-100 values determining the randomness of the next node
/// @param pheromone_param how much pheromone trail affects the randomness
/// @param heuristic_param how much heuristic information affects the randomness
/// @param visited_c vector<bool> telling how many customers the current ant has visited
/// @param current_time time in minutes
/// @param speed in units/minute
/// @param rng random number generator
/// @param w1 the parameter telling how edge length and edge time window is affecting the node selection.
/// @return int, the index of the next node in customers vector.
int NextNode(Customer &i, vector<Customer> &customers, const vector<float> &pheromoneMap,
             int r0, float pheromone_param, float heuristic_param, vector<bool> &visited_c, float current_time,
             float speed, std::mt19937 rng, float w1)
{

    // Create a uniform distribution over the specified range
    std::uniform_int_distribution<int> random_r(0, 100);

    // random number generator between 0-100
    int r = random_r(rng);

    // check if the answer should be get from the heuristics or randomly
    if (r <= r0)

    {
        // float to store the biggest heuristic value and customer it belongs to
        float biggest_heuristic_value = -0.5;
        int heuristic_node_id = -1;

        for (int n = 0; n < visited_c.size(); ++n) // loops through nodes that hasn't been visited
        {

            if (!visited_c[n]) // checks the heuristic value for nodes that has not been visited yet
            {

                float heuristic_value = i.HeuristicValue(current_time, customers[n], speed, w1);
                if (heuristic_value > -0.5) // if it was possible to travel to customers[n]
                {
                    float pheromone = pow(pheromoneMap[n], pheromone_param);
                    float heuristic = pow(heuristic_value, heuristic_param);

                    // if new heuristic * pheromone value was bigger than previously biggest, update the biggest value
                    if (pheromone * heuristic > biggest_heuristic_value)
                    {
                        biggest_heuristic_value = pheromone * heuristic;
                        heuristic_node_id = n;
                    }
                }
            }
        }

        // no node found where moving is possible if heuristic_node_id = -1
        return heuristic_node_id;
    }

    // calculate the next random node from a weighted distribution where the heuristic * pheromone values are the weights

    vector<float> heuristic_prob(customers.size(), 0);
    float heuristic_sum = 0;

    for (int n = 0; n < visited_c.size(); ++n) // loops through nodes that hasn't been visited
    {
        if (!visited_c[n]) // checks the heuristic value for nodes that has not been visited yet
        {
            float heuristic_value = i.HeuristicValue(current_time, customers[n], speed, w1);
            if (heuristic_value > -0.5)
            {
                float pheromone = pow(pheromoneMap[n], pheromone_param);
                float heuristic = pow(heuristic_value, heuristic_param);

                heuristic_prob[n] = (pheromone * heuristic);
                heuristic_sum += pheromone * heuristic;
            }
        }
    }

    // if no nodes were possible to go to
    if (heuristic_sum == 0)
    {
        return -1;
    }

    // create a vector where the amounts of the nodes are weighted with heuristic_prob
    vector<int> next_random_node = {};
    for (int d = 0; d < heuristic_prob.size(); ++d)
    {

        heuristic_prob[d] = (heuristic_prob[d] / heuristic_sum) * 100;
        for (int k = 0; k < heuristic_prob[d]; ++k)
        {
            next_random_node.push_back(d);
        }
    }

    // Create a uniform distribution over the specified range
    std::uniform_int_distribution<int> random_node_index(0, next_random_node.size() - 1);

    int node_index = random_node_index(rng);

    return next_random_node[node_index];
}

/// @brief updates the pheromone trails
/// @param path is the path where pheromone trails should be increased
/// @param pheromone_map map of all the pheromone edges
/// @param distance is the length of the path
/// @param evaporation is the evaporation factor between 0 and 1. Higher values also increases the pheromone trail more
void UpdatePheromone(const vector<vector<int>> &path, vector<vector<float>> &pheromone_map,
                     float distance, float evaporation)
{

    // evaporate all the values
    for (vector<float> &row : pheromone_map)
    {
        for (float &value : row)
        {
            value = (1 - evaporation) * value;
        }
    }

    // increase all pheromone edges of path
    float lambda = (evaporation / distance);
    int i = 0;
    for (int r = 0; r < path.size(); r++)
    {
        i = path[r][0]; // i is the depot

        // updates the pheromone path for the first edge of the path
        pheromone_map[pheromone_map.size() - 1 - i][path[r][1]] += lambda;

        i = path[r][1];

        // updates the rest of the path
        for (int c = 2; c < path[r].size(); c++)
        {
            pheromone_map[i][path[r][c]] += lambda;
            i = path[r][c];
        }
    }
}

/// @brief Calculates the paths for customers in customers.csv and depots in depots.csv
/// @param r_0 value determining the randomness of the next node 100: no randomness. 0: only randomness
/// @param pheromone_init  deafault value of pheromonetrails
/// @param number_of_ants Smaller numbers speed up the process, but routes may be longer
/// @param pheromone_param how much pheromone trail affects the randomness
/// @param heuristic_param how much heuristic information affects the randomness
/// @param evaporation_factor how much the pheromone weakens during an iteration
/// @param g_evaporation_factor how much the pheromone weakens between iterations
/// @param iterations Smaller numbers speed up the process, but routes may be longer
/// @param w1 Value between 0-1. its the parameter telling how edge length and edge time window is affecting the node selection.
/// @return sum of the paths
float Run(int r_0, float pheromone_init, int number_of_ants, float pheromone_param, float heuristic_param, float evaporation_factor, float g_evaporation_factor, int iterations, float w1)
{

    // create the vector of customers
    vector<Customer> customers = readDataFromCSV("customers.csv", 20);
    int number_of_customers = customers.size();

    unsigned seed = static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count());

    // Create a random number generator engine
    std::mt19937 rng(seed);

    vector<Customer> depots = readDataFromCSV("depots.csv");
    int number_of_depots = depots.size();

    // Create a uniform distribution over the specified range
    std::uniform_int_distribution<int> random_depot(0, number_of_depots - 1);

    float speed = 20; // speed of the vehicles as units / minute
    float current_time = 0;

    vector<vector<int>> best_route = {};
    float best_length = -1;

    auto pheromone_map = CreatePheromoneMap(number_of_customers, number_of_depots, pheromone_init);

    int number_of_iteration = 0;

    int counter = 0;
    while (counter < iterations)
    {
        int k = 0; // current ant number

        while (k < number_of_ants) // step 2.
        {

            vector<bool> visited_customers(number_of_customers, false); // step 3.
            int visited_number = 0;

            int start_depot = random_depot(rng);
            int i = start_depot; // step 4

            // makes a route where everyone gets visited
            vector<vector<int>> route = {};
            route.push_back({i});
            int route_length = 0;

            int n_of_vehicle = 0;
            int j;

            int soft_lock_counter = 0;

            try
            {
                while (visited_number < number_of_customers) // TODO: prevention of soft lock when there is a customer that cant be reached from any depot

                {

                    if (route[n_of_vehicle].size() <= 1)
                    {

                        j = NextNode(depots[i], customers, pheromone_map[pheromone_map.size() - 1 - i],
                                     r_0, pheromone_param, heuristic_param, visited_customers, current_time, speed, rng, w1);
                        for (int depot = 0; depot < depots.size(); depot++)
                        {
                            if (depots[depot].DistanceTo(customers[j]) < depots[route[n_of_vehicle][0]].DistanceTo(customers[j]))
                            {
                                route[n_of_vehicle][0] = depot; 
                            }
                        }
                    }
                    else
                    {

                        j = NextNode(customers[i], customers, pheromone_map[i],
                                     r_0, pheromone_param, heuristic_param, visited_customers, current_time, speed, rng, w1);
                    }

                    if (j < 0) // if no possible next node was found
                    {
                        if(soft_lock_counter > 20)
                        {
                            cout << "Not possible to reach a customer" << endl;
                            return 0;
                        }

                        soft_lock_counter++;

                        i = random_depot(rng);

                        if (route[n_of_vehicle].size() == 1)
                        {
                            route[n_of_vehicle][0] = i;
                        }
                        else
                        {
                            n_of_vehicle++;
                            route.push_back({i});
                        }
                        current_time = 0;
                        continue;
                    }

                    soft_lock_counter = 0;
                    route[n_of_vehicle].push_back(j); // put edge i-j to the route
                    visited_number++;
                    visited_customers[j] = true;

                    number_of_iteration++;

                    // check for timing and add time accordingly to current_time
                    if (route[n_of_vehicle].size() > 1)
                    {
                        current_time += customers[i].TimeToJ(customers[j], speed) + customers[i].getServiceTime();
                        route_length += customers[i].DistanceTo(customers[j]);
                    }
                    else
                    {
                        current_time += depots[i].TimeToJ(customers[j], speed);
                        route_length += depots[i].DistanceTo(customers[j]);
                    }
                    if (customers[j].getStartTime() > current_time)
                    {
                        current_time = customers[j].getStartTime();
                    }
                    i = j;
                }
            }
            catch (...)
            {
                std::cout << "exeption caught" << std::endl;
            }

            UpdatePheromone(route, pheromone_map, route_length, evaporation_factor); // update pheromone locally

            if (route_length < best_length || best_length < 0) // saves the new shortest path
            {
                best_route.clear();
                for (int r = 0; r < route.size(); r++)
                {
                    best_route.push_back({});
                    for (int node : route[r])
                    {
                        best_route[r].push_back(node);
                    }
                }
                best_length = route_length;
            }

            k++;
        }

        UpdatePheromone(best_route, pheromone_map, best_length, g_evaporation_factor);

        // std::cout << "length: " << best_length << " Vehicles: " << best_route.size() << endl;
        //  TODO: if breaking condition is met exit the loop and save the best route
        //  step 8.
        counter++;
    }

    std::cout << "finished. Length: " << best_length;
    std::cout << " vehicle counter: " << best_route.size() << endl;

    savePathsToCSV(best_route);
    return best_length;
}

int main()
{

    int r_0 = 85;             // 85 tested. value determining the randomness of the next node 100: no randomness. 0: only randomness
    float pheromone_init = 5; // 5 tested. deafault value of pheromonetrails

    int number_of_ants = 15;          // 15 tested. number of ants
    float pheromone_param = 1;        // 1 tested. how much pheromone trail affects the randomness
    float heuristic_param = 10;       // 14 tested. how much heuristic information affects the randomness
    float evaporation_factor = 0.07;  // 0.07 tested. Value between 0-1. how much the pheromone weakens during an iteration
    float g_evaporation_factor = 0.7; // 0.7 tested. Value between 0-1. how much the pheromone weakens between iterations
    int iteration = 700;              // 700 tested. Smaller numbers speed up the process, but routes may be longer
    float w1 = 0.3;                   // 0.1 tested. Value between 0-1. its the parameter telling how edge length and edge time window is affecting the node selection.

    float route_length = Run(r_0, pheromone_init, number_of_ants, pheromone_param, heuristic_param, evaporation_factor, g_evaporation_factor, iteration, w1);

    return 0;
}
