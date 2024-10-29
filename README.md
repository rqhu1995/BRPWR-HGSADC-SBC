# BRPWR-HGSADC-SBC

The repository provides the data and codes to conduct the experiments in the paper "Repositioning in bike sharing systems with broken bikes considering on-site repairs". Specifically, the repository implements the **Hybrid Genetic Search Adaptive Diversity Control and Station Budget Constrained Heuristic (BRPWR-HGSADC-SBC)** algorithm to solve the **Bike Repositioning Problem with on-site Repairers**.

## Project Structure

- **Program/**: Contains the main C++ source code for the BRPWR-HGSADC-SBC algorithm.
  - `Genetic.cpp`, `Genetic.h`: Implements the genetic algorithm for route optimization.
  - `Individual.cpp`, `Individual.h`: Defines the individuals in the population used by the genetic algorithm.
  - `Instance.cpp`, `Instance.h`: Manages instances of the bike repositioning problem.
  - `LocalSearch.cpp`, `LocalSearch.h`: Implements the local search procedures for optimization.
  - `Params.cpp`, `Params.h`: Defines various parameters for the genetic algorithm.
  - `Population.cpp`, `Population.h`: Handles the population evolution process.
  - `RepositionScheme.h`, `Station.h`: Defines the repositioning scheme and station properties.
  - `main.cpp`: The main entry point for running the algorithm.
- **CMakeLists.txt**: Configuration file for building the project using CMake.
- **run.sh**, **runall.sh**, **prop.sh**: Shell scripts to run experiments and automate different scenarios.
- **LICENSE**: License file for the repository.
- **README.md**: This file, providing an overview of the project.

## Data description and usage

### Data naming and format description

The folders `X_Y` provide data for different sizes (`X` stations). `Y` here is the instance number. Inside each folder `X_Y`, we have:

- `dissat_table_i.txt (i = 1, 2, ..., X)`: These are the extended user dissatisfaction table calculated with a size of $C_i \times C_i$, where $C_i$ is capacity of station $i$. The row index $p$ is the number of usable bikes, and the column index $b$ is the number of broken bikes at station $i$. For table cells $(p,b)$ where $p+b > C_i$, the values are uniformly set as 0.
- `linear_diss_i.txt (i = 1, 2, ..., X)`: These are the linearized user dissatisfaction values for station $i$. The values are calculated based on the dissatisfaction tables and the method described in the paper. Each line inside the file represents one combination of $p$ and $b$ sorted in the order of $p$ fixed and $b$ varying. The values are separated by commas as $p$, $b$, $\alpha$, $\beta$, $\gamma$, and $s$, where $\alpha$, $\beta$, and $\gamma$ are the coefficients in the linearized function, and $s$ is the deviation of the linearized function from the original EUDF value.
- `BCRFT_i.txt (i = 1, 2, ..., X)`: These are the beneficial over cost ratio function for trucks at station $i$. The values are calculated based on the method described in the paper. This is a table similar to the dissatisfaction table, except for the values are the beneficial over cost ratio for each combination of $p$ and $b$.
- `BCRFR_i.txt (i = 1, 2, ..., X)`: These are the beneficial over cost ratio function for repairers at station $i$. The values are calculated based on the method described in the paper. This is a table similar to the dissatisfaction table, except for the values are the beneficial over cost ratio for each combination of $p$ and $b$.
- `station_info_X.txt`: This file contains the information of stations inside an `X`-station BSS network. The first line is the header, and the following lines are the information of each station. The information includes the station id, the capacity of the station, the number of current usable bikes at the station, the number of target usable bikes at the station, and the number of current broken bikes at the station. The values are separated by `\t`. The station_id here is the station id in the original dataset, after the data is read, they are re-indexed to `1,2,...,X`.
- `time_matrix_X.txt`: This file contains the time matrix between stations inside an `X`-station BSS network. The unit is seconds. Note depot is included in the station list, so the size of the matrix is $(X+1) \times (X+1)$. The values are separated by `\t`. Also note the matrix is asymmetric, i.e., the time from station $i$ to station $j$ is not necessarily the same as the time from station $j$ to station $i$.

### Data specification on the correspondence to experiments in the paper

- `X_Y` with `X = 6, 10, 15` and `Y = 1, 2, 3, 4, 5` correspond to the experiments on small scale instances in Section 5.2 of the paper.
- `X_Y` with `X = 60, 90, 120, 200, 300, 400, 500` and `Y = 1` correspond to the experiments on large scale instances in Section 5.3 of the paper.

### Using the data

To use the data,  [pandas](https://pandas.pydata.org/) library is recommended. The following code snippet shows how to read the data:

```python
import pandas as pd
df_station_info = pd.read_csv('Instances/6_1/station_info_6.txt', sep='\t', header=0)
df_time_matrix = pd.read_csv('Instances/6_1/time_matrix_6.txt', sep='\t')
```

## Dependencies

To compile and run the project, you need:

- **C++ Compiler** (e.g., GCC, Clang)
- **CMake** (version 3.17 or higher)
- **Boost Libraries** (for handling various operations within the code)

## Building the Project

To build the project, run the following commands from the root directory:

```sh
mkdir build
cd build
cmake ..
make
```

This will create an executable that can be used to run the bike repositioning optimization.

## Running the Project

The project can be run with various configurations using the command-line arguments. Example usage:

```sh
./main -S 6 -K 1 -R 1 -NT 12 -tau 600 -Q 25 -tl 60 -tr 300 -r True -i 1 -e default -t 4 -heu 0.00 -m time_limit
```

### Command-Line Arguments

| Argument            | Description                                                      | Default Value  |
|---------------------|------------------------------------------------------------------|----------------|
| `-h`, `--help`      | Show this help message and exit                                  | `N/A`           |
| `-ns`, `--num_stations` | Number of stations                                            | `10`           |
| `-ntrk`, `--num_trucks`  | Number of trucks                                             | `1`            |
| `-nrpm`, `--num_repairer` | Number of repairers                                         | `1`            |
| `-i`, `--inst_no`   | The instance ID                                               | `1`            |
| `-ldT`, `--loading_time` | Loading time                                                | `60`           |
| `-rpT`, `--repair_time`  | Repair time                                                 | `300`          |
| `-vcap`, `--vehicle_capacity` | Vehicle capacity                                      | `25`           |
| `-tb`, `--time_budget` | Time budget for the repositioning                            | `-1.0`         |
| `-pnt`, `--penalty`  | Initial penalty for each violation of the constraints         | `10`           |
| `-noimp`, `--num_non_improve` | Number of iterations without improvement                | `5000`         |
| `-num_pnt_manage`, `--num_penalty_management` | Number of iterations for penalty management | `100`          |
| `-fesP`, `--target_feasible` | Target feasible solution percentage in the population  | `0.2`          |
| `-tl`, `--timeLimit` | Time limit for the algorithm as one of the terminating criteria | `7200.0`    |
| `-mu`, `--mu`        | Mu                                                           | `25`           |
| `-lambda`, `--lambda` | Lambda                                                      | `40`           |
| `-edu`, `--itedu`    | Number of iterations repeated when an operator improved a solution | `40`      |


To facilitate the testing of the results, we wrote scripts for automating the execution process for both small and large instances. To use the scripts, simply execute

```bash
./large.sh 10800 # or 7200, the number means the time budget in seconds
```

or

```bash
./small.sh
```

Note that for large instances, we utilized `GNU screen` to attach the process in the background and better manage the results. Hence [the installation of `screen`](https://www.gnu.org/software/screen/) is required to use the script `large.sh`.

## Check the solution
Solutions will be saved in the folder `Solutions`, and the name of the solution file is `<number of stations>_<instance no.>_t<truck number>_r<repairer number>_<time budget in hours>h_<running time>`, with the content be like:

```
the best solution is: 
individual's fitness value: 484.538
individual's dissat value: 241.776
individual's emission value: 16.4385
individual's trkRoute: 3496.6
individual's rpmRoute: 1900.25
individual's trkOperationTime: 9480
individual's rpmOperationTime: 5100
the repositioning scheme for truck is: 
=======
0	load 13 usable bikes;load 0 broken bikes;unload 0 usable bikes;unload 0 broken bikes
23	load 11 usable bikes;load 0 broken bikes;unload 0 usable bikes;unload 0 broken bikes
17	load 0 usable bikes;load 10 broken bikes;unload 24 usable bikes;unload 0 broken bikes
0	load 0 usable bikes;load 0 broken bikes;unload 0 usable bikes;unload 10 broken bikes
=======
0	load 23 usable bikes;load 0 broken bikes;unload 0 usable bikes;unload 0 broken bikes
20	load 0 usable bikes;load 11 broken bikes;unload 20 usable bikes;unload 0 broken bikes
19	load 0 usable bikes;load 2 broken bikes;unload 3 usable bikes;unload 0 broken bikes
26	load 9 usable bikes;load 0 broken bikes;unload 0 usable bikes;unload 0 broken bikes
0	load 0 usable bikes;load 0 broken bikes;unload 9 usable bikes;unload 13 broken bikes
the repositioning scheme for repairman is: 
=======
0	repair 0 bikes
18	repair 4 bikes
16	repair 2 bikes
5	repair 1 bikes
10	repair 5 bikes
28	repair 5 bikes
0	repair 0 bikes

CPU time: 19.3971
dissat at each station
dissat[1] = 8.48468
dissat[2] = 5.58259
dissat[3] = 0.817082
dissat[4] = 2.64005
dissat[5] = 9.61442
dissat[6] = 1.81396
dissat[7] = 6.9177
dissat[8] = 1.21141
dissat[9] = 7.32871
dissat[10] = 7.82354
dissat[11] = 7.50517
dissat[12] = 5.89359
dissat[13] = 0.193163
dissat[14] = 4.96948
dissat[15] = 12.3184
dissat[16] = 13.0506
dissat[17] = 10.9077
dissat[18] = 8.85136
dissat[19] = 10.8172
dissat[20] = 7.4787
dissat[21] = 3.59874
dissat[22] = 6.20671
dissat[23] = 7.93232
dissat[24] = 9.06817
dissat[25] = 20.3564
dissat[26] = 15.7174
dissat[27] = 6.73404
dissat[28] = 9.29928
dissat[29] = 9.43885
dissat[30] = 19.2044
```