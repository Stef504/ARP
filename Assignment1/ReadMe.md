ReadMe

What each process does:

Main(Master):
This is the master process responsible for creating the concurrent program. It uses the primitive fork() and execlp() to execute each process. Other primitive uesed are pipes() this allows for the communication between processes. The respective pipes (open or closed) are stated with each fork. The wait primitive is used at the end of the code to ensure that all parents wait for their child to finish before returning.  
Main is then responsible for executing the blackboard, the drone physics, the input handler, the obstacles and the target generation.

The Black Board:
This is the proccess for inter process communication between all processes. All information of the targets, obstacles, drone physics and input is sent to the black board so it can share certain messages to respective processes. This is all done through the intialization of pipes which were given from command line arguments (sent from Main) and the primitive select().
The primitive select() allows us to avoid race-condition. Pipe() allows for us to read/write data to a specific pipe.

The drone physics communicates its position to the black board. The blackboard will move the drone on the window.
The black board will communicate to the drone if the drone is near an obstacle.
The obstacles and targert generation send its coordinates to the black board to draw on the window and identify if the drone is in proximity to the drone.
Input commands like 'a','q','u' are sent to the blackboard via a named pipe. 
Reads from parameter file

Drone Physics:
Calculates the drone physics (movement and repulsion forces) and sends its coodinates to the black board. It initializes which pipes it needs which are given from the command line arguments (send from main) and uses the primitive select().

Input:
Uses the signal() primitive to ensure that if the input pipe is broken/closed it will not terminate the program immediately. Instead it ignores the SIGPIPE and allows the client the handle the error to ensure restore the canonical mode. 

Obstacle and Target Generation: 
Reads from the parameter file, intializes pipes which were given from command line arguments (sent from Main) and writes the co-ordinates every 7s to the black board. Its random generation based off the time and PID so that its sequence is unique. 

List of Components, Directories, Files:
/Assignment1
|-main.c
|-BlackBoard.c
|-process_Drone.c
|-process_In.c
|-process_Ta.c
|-process_Ob.c
|-Parameter_File.txt
|-ReadMe.md
|-TODO

Makefile:


