## 2. Active Components

### 🖥️ Master Process (`main.c`)

This is the master process tasked with generating the concurrent program. It employs the primitive fork() and execlp() functions to initiate each process. Additional primitives employed include pipes(), which assist in the communication between processes. Pipe() also enables the reading and writing of data to a designated pipe. The corresponding pipes (opening or closing) are indicated at each fork and all pipes are closed at the end of the code to ensure the program does not freeze / enter deadlocking. The wait primitive is employed at the end of the code to guarantee that all parent processes await the completion of their child processes before returning.

Main is responsible for executing the blackboard, drone physics, input handling, and obstacle and target generation.

The use of unnamed pipes() allows for fast and simple communication between related processes. While the use of the named pipes (FIFOs) allows for communication between unrelated processes.

### 📊 Server / Blackboard (`BlackBoard.c`)
This outlines the procedure for interprocess communication among all processes. All data on targets, obstacles, drone physics, and inputs is transmitted to the blackboard to facilitate the broadcasting of specific data to the corresponding processes. The process is executed via the initialisation of pipes provided through command line parameters (transmitted from Main) and the basic select() function.

The basic select() function enables the prevention of race conditions. 

Key algorithm of the blackboard:
- The blackboard additionally retrieves information from the parameter file.
- The window adjusts based on the user's preferences; however, each adjustment causes a reset of the drone's position.
- The drone's physics module transmits its location to the blackboard. The blackboard will display the drone's location on the window.
- The blackboard will notify the drone of its proximity to obstacles, its initial location at the game's starting point, and when the drone is restored to the home position.
- The obstacles and target generation transmit their coordinates to the blackboard to render on the window and establish if the drone is near the obstacle.
- Commands such as 'a', 'q', and 'u' are transmitted to the blackboard via a named pipe.
- Obstacles and targets are generated dynamically following a cumulative total of 20 generations.
- Obstacles, targets and the drone is clamped to the window size to adhere to the game boundaries


### 🕹️ Drone Process (`process_Drone.c`)
The drone's physics (motion and repulsive forces) are computed and its coordinates are transmitted to the blackboard. It initialises the requisite pipes, as specified by the command line arguments provided by the main function, and utilises the primitive select() for reading from the blackboard and input processes.

### Input Process (`process_In.c`)
It employs the signal (SIGPIPE, SIG_IGN) mechanism to guarantee that the program does not terminate immediately if the input pipe is disrupted or closed. Instead, it disregards the SIGPIPE and allows the client to manage the error to ensure the resetting of the canonical mode. 

The process sends the user's input directly to the drone and the blackboard over distinct channels. The modification of the canonical mode removed the necessity for the user to press ENTER after each input.

### 🚧 Obstacle and 🎯 Target Generator(`process_Ob.c`, `process_Ta.c`)
The process reads the parameter file, initialises pipes provided through command-line options (transmitted from Main), and publishes coordinates. They are also clamped to the current window size and adjust accordingly.
Target coordinates are generated and published every 7 seconds on the blackboard. While obstacle coordinates are generated and publsihed every 5 seconds to the blackboard. 
The different timings ensure that generations do not appear at once. The random generation relies on the time and PID, ensuring a unique sequence for generation.

## 3. List of Files
/Assignment1
|-main.c
|-BlackBoard.c
|-process_Drone.c
|-process_In.c
|-process_Ta.c
|-process_Ob.c
|-Parameter_File.txt
|-ReadMe.md
|-Makefile
|-TODO

## 🛠️ Installation and Running

**1. Prerequisites**
Ensure all source files (`.c`) and the `Makefile` are in the same directory.

**2. Compilation**
Open your terminal in the project folder and run the build script:
```bash
make
./main
```

## 🕹️Operational Instructions
'e' - moves up
'c' - moves down
'f' - moves right
's' - moves left
'r' - moves north-eat
'x' - moves south-west
'w' - moves west-north
'v' - moves east-south

'd' - breaks
'a' - reset
'q' - quits the game
'p' - pauses the game
'u' - unpauses the game






