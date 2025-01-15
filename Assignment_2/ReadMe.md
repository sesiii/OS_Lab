# CS39002 Operating Systems Laboratory
## Spring 2025
### Lab Assignment: 2

**Date of submission:** 15–Jan–2025

---

## Inter-process Communication using Signals

### Problem Statement
This assignment involves simulating a game where a parent process (P) and `n` child processes (C1, C2, ..., Cn) communicate using signals. The parent throws a ball to the children in a circular order. If a child catches the ball, they continue to play; if they miss, they are out of the game. The game continues until only one child remains.

### Files
- `parent.c`: Simulates the parent process.
- `child.c`: Simulates each child process.
- `dummy.c`: Simulates a dummy process used for synchronization.
- `makefile`: Compiles the programs.

### Approach

#### parent.c
1. **Initialization**:
    - The parent process creates `n` child processes.
    - Writes the PIDs of the child processes to `childpid.txt`.

2. **Game Loop**:
    - Starts the game by sending `SIGUSR2` to the first child.
    - Waits for signals from children to determine if they caught or missed the ball.
    - Records the status of each child.
    - Initiates status printing by sending `SIGUSR1` to the first child.
    - Waits for the dummy process to exit before making the next throw.
    - Continues until only one child remains.

3. **Termination**:
    - Sends `SIGINT` to all child processes.
    - The last remaining child prints a happy message and exits.

#### child.c
1. **Initialization**:
    - Each child reads `childpid.txt` to get the PIDs of all child processes.
    - Enters an infinite loop, waiting for signals.

2. **Signal Handling**:
    - On receiving `SIGUSR2`, decides randomly whether to catch or miss the ball.
    - Sends `SIGUSR1` to the parent if the ball is caught, or `SIGUSR2` if missed.
    - On receiving `SIGUSR1`, prints its current status and sends `SIGUSR1` to the next child.

3. **Termination**:
    - Exits on receiving `SIGINT`.

#### dummy.c
1. **Initialization**:
    - Enters an infinite loop of `pause()` at the beginning of its `main()`.
    - Exits on receiving `SIGINT` from the last child after status printing.

### Compilation and Execution
Use the provided `makefile` to compile and run the programs:
```sh
make all
./parent 10
```

### Cleanup
To clean up the generated files, use:
```sh
make clean
```

### Submission
Submit a zip/tar/tgz archive containing the files `parent.c`, `child.c`, `dummy.c`, and `makefile`.

---

**Note**: Ensure to follow the specified format for printing the status of the child processes and use `fflush(stdout);` to avoid garbled output.