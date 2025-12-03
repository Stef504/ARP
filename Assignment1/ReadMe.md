## Architecture Sketch
[📄 Click here to view the sketch of the architecture (PDF)](./Sketch_of_Architecture.pdf)

## Active Components

### 🖥️ Master Process (`main.c`)

This is the master process tasked with generating the concurrent program. It employs the primitive fork() and execlp() functions to initiate each process. Additional primitives employed include pipes(), which assist in the communication between processes. Pipe() enables the reading and writing of data to designated pipes. The corresponding pipes (opening or closing) are indicated at each fork and all pipes are closed at the end of the code to ensure the program does not freeze / enter deadlocking. The wait() primitive is employed at the end of the code to guarantee that all parent processes await the completion of their child processes before returning. 

The kill(*process_name*, SIGTERM) signal sends signals to all other processes to terminate. This is executed when the drone process exits. If this signal did not work, the kill(*process_name*, SIGKILL) signal will then forcefully kill all other processes. This primitive is important in shutting down the entire code smoothly.

The respective **POSIX Macros** used in Main are: 
    - `WIFEXITED` - used to identify how each child process was terminated
    - `WEXITSTATUS` - used to extract the type of exit code 
    - `WIFSIGNALED` - if the child process was terminated by a signal

Main is responsible for executing the blackboard, drone physics, input handling, and obstacle and target generation.

The use of unnamed pipes() allows for fast and simple communication between related processes. While the use of the named pipes (FIFOs) allows for communication between unrelated processes.

### 📊 Server / Blackboard (`BlackBoard.c`)
This outlines the procedure for interprocess communication among all processes. All data on targets, obstacles, drone physics, and inputs is transmitted to the blackboard to facilitate the broadcasting of specific data to the corresponding processes. The process is executed via the initialisation of pipes provided through command line parameters (transmitted from Main) and the basic select() function.

The basic select() function enables the prevention of race conditions. 

- The data accepted from the **fdToBB** pipe is as listed: 
    -The drones current coordinates

- The data sent from the **fdFromBB** pipe is as listed: 
    - *refer to the Drone Process*

- The data accepted from **fdIn_BB** named pipe is as listed:
    - Commands such as 'a', 'q', and 'u'. *see operation instructions* 

- The data accepted from the **fdOb, fdTa** pipes are as listed:
    - Coordinates of obstacles and targets

Key algorithm of the blackboard:
- The blackboard additionally retrieves information from the parameter file.
- It initialises the requisite pipes, as specified by the command line arguments provided by the main function
- The window adjusts based on the user's preferences; however, each adjustment causes a reset of the drone's position.
- The drone's physics module transmits its location to the blackboard. The blackboard will display the drone's location on the window.
- The window is redrawn every iteration.
- The blackboard will notify the drone of its proximity to obstacles, its initial location at the game's starting point, and when the drone is restored to the home position.
- The obstacles and target generation transmit their coordinates to the blackboard to render on the window and establish if the drone is near the obstacle.
- Obstacles and targets are generated dynamically following a cumulative total of 20 generations.
- Obstacles, targets and the drone is clamped to the window size to adhere to the game boundaries
- The system utilises standard exit codes, and upon completion of the code, it closes all associated pipes.


### 🕹️ Drone Process (`process_Drone.c`)
- The drone's physics (motion and repulsive forces) are computed. 
- It initialises the requisite pipes, as specified by the command line arguments provided by the main function, and utilises the primitive select() for reading from the blackboard and input processes.
- The drone will move continously in one direction until either of the conditions are met:
    1. Brakes are applied
    2. Rest is applied
    3. An opposing direction was entered
- The speed of the drone increases in increments of 20%. A maximum of two boosts are applied. This relates to three consecutive clicks of the same letter. The drone can only slow down in the current direction if its opposite button is pressed 3 times. 

- The data accepted from the BlackBoard (**fdFromBB** pipe) is as listed: 
    - The intial position of the drone when the game is launch
    - Clamping of the drone when it reaches the boarder of the window to prevent vanishing
    - When the user resizes the window the drone is reset to the center 
    - When the user resets the drone

- The data sent to the BlackBoard (**fdToBB** pipe) is as listed:
    - Its current coordinates after the respected forces have been calculated

- The date accepted from the BlackBoard via the **fdRepul** pipe is as listed:
    - Data is only sent when the distance between the obstacle and the drone is below rho. The distance calculation is done in the Blackboard.
    - This function enables the calculation and addition of the repulsive forces. 
    - When the drone is notified about the data received, it turns off any boosts and removes the active key.
    - To visualise the repulsive force, a scaling factor has been applied and a maximum force has been implemented to reduce high velocity when being repelled. 

- The data received from the input process via the **fdIn** pipe is as listed:
    - The coordinates to control the drone

- Safety Factors:
    - The signal (SIGPIPE, SIG_IGN) mechanism to guarantee that the program does not terminate immediately if the input pipe is disrupted or closed
    - The data written to the BlackBoard and if it failed to write
    - The distance the repulsion force reacted, and the amount of force applied
    - The system utilises standard exit codes, and upon completion of the code, it closes all associated pipes.


### Input Process (`process_In.c`)
- This records the users input. Only the designated input commands will cause a reaction to both the drone and the blackboard.
- The process sends the user's input directly to the drone and the blackboard over distinct channels (**fdIn** and **fdIn_BB**). - The modification of the canonical mode removed the necessity for the user to press ENTER after each input.

Safety Factor:
- Signal (SIGPIPE, SIG_IGN) mechanism to guarantee that the program does not terminate immediately if the input pipe is disrupted or closed. Instead, it disregards the SIGPIPE and allows the client to manage the error to ensure the resetting of the canonical mode. 
- The system utilises standard exit codes, and upon completion of the code, it closes all associated pipes.


### 🚧 Obstacle and 🎯 Target Generator(`process_Ob.c`, `process_Ta.c`)
- The process reads the parameter file, initialises pipes provided through command-line options (transmitted from Main), and publishes coordinates. 
- The generation is clamped to the current window size but is adjusted accordingly in the BlackBoard.
- Target coordinates are generated and published every 7 seconds on the blackboard. 
- While obstacle coordinates are generated and publsihed every 5 seconds to the blackboard. 
- The system utilises standard exit codes, and upon completion of the code, it closes all associated pipes.

The different timings ensure that generations do not appear at once. The random generation relies on the time and PID, ensuring a unique sequence for generation. 
The readjustment of obstacles and targets as the window changes allows for an even spread of obstacles and targets.

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
|-Sketch_of_Architecture.pdf

## 🛠️ Installation and Running

**1. Prerequisites**
Ensure all source files (`.c`) and the `Makefile` are in the same directory.

**2. Compilation**
Open your terminal in the project folder and run the build script:
```bash
make
./main
```
To clear executables, open your terminal in the project folder and run the following script:
```bash
make clean
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

> **⚠️ Disclaimer:**
> Please wait for the drone to stop moving after the repulsion force was detected so as to provide the program enough time to react before pressing another key.

### Necessary Notes:
All commits are accessible for evaluation on GitHub at https://github.com/Stef504/ARP.git.

These commits demonstrate our continued progress on the assignment. This project is a work in progress, primarily aimed towards ensuring that the interprocess communication via pipes is efficient and synchronised. Our second objective was to verify the accuracy of the computational logic and mathematics defining the obstacle and target generation, as well as the drone physics. The game's graphics will be enhanced during the project's final phases. We sincerely value your feedback to further our coding skills. 






