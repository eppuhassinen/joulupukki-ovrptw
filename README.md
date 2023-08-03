# joulupukki-mdovrptw
This is a student project experimenting with ant-colony-optimization and vehicle routing problem.

The Run() function in main.cpp calculates a route for MDOVRPTW (multi-depot open vehicle routing problem with time windows).

# Depots
depots.csv contains all the depots where routes can begin.

if the croodinates of three depots are: d0 = (1, 1), d1 = (100, 2), d3 = (300, 100).
depots.csv should be formatted:

1;1;0;0 // this is depot number 0

100;2;0;0 //this is depot number 1 etc.

300;100;0;0

each row represents one depot and must have two zeros at the end because depots are treated as customers.

# Customers

customers.csv contains all the customers that needs to be visited.

A row in customers.csv file is one customer that is formatted: x_coord;ycoord;time_window_start;time_window_end

Times are integer values representing minutes from start time. Start time is defaulted as 0. Vehicles default speed is 20 units/minute.

# Routes
Generated routes will be saved to a routes.csv file. Each row is a different vehicle's route. First colum is the starting depot number and the other columns are the customer numbers.

# plotter.mlx
plotter.mlx is Matlab live script visualizing the generated routes.
