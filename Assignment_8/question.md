# Banker's Algorithm Assignment Breakdown


## Milestone 1: Understanding the Problem

1. **Assignment Overview**:
   - Implement a multi-threaded application that simulates resource allocation with potential deadlocks
   - Use the banker's algorithm for deadlock avoidance
   - Have 'm' resource types (5-20) and 'n' threads (10-100)
   - Support two versions: one that allows deadlocks and one that avoids them

2. **Key Concepts**:
   - **Banker's Algorithm**: A deadlock avoidance algorithm that checks if a system is in a safe state before granting resources
   - **Resource Types**: Different kinds of resources each with multiple instances
   - **Thread Requests**: Threads can request or release resources
   - **Safe State**: A state where all threads can eventually complete execution

## Milestone 2: Input Handling

1. **Directory Structure**:
   - Create an `input/` directory to store input files
   - Parse `system.txt` for system parameters (m, n, and available resources)
   - Parse thread files (`thread00.txt`, `thread01.txt`, etc.) for thread resource requests

2. **Understanding Request Types**:
   - **RELEASE**: All request values are ≤ 0 (release or maintain resources)
   - **ADDITIONAL**: At least one request value > 0 (requesting additional resources)
   - **QUIT**: Final request in each thread file (thread releases all resources and terminates)

3. **Request Format**:
   - `DELAY R REQ[0] REQ[1] ... REQ[m-1]`: Regular resource request
   - `DELAY Q`: Quit request (thread terminates after DELAY)

## Milestone 3: Data Structures Design

1. **System Matrices/Vectors**:
   - `AVAILABLE[m]`: Available instances of each resource type
   - `MAXIMUM[n][m]`: Maximum demand of each thread for each resource
   - `ALLOCATION[n][m]`: Current allocation for each thread
   - `NEED[n][m]`: Remaining need for each thread (`MAXIMUM - ALLOCATION`)

2. **Request Queue**:
   - Implement a FIFO queue to store pending ADDITIONAL requests
   - Store pairs of (thread_id, request_vector) in the queue

3. **Synchronization Tools**:
   - Barriers for coordinating thread starts and request communications
   - Mutexes for protecting shared data
   - Condition variables for blocking/unblocking threads

## Milestone 4: Thread Management

1. **Master Thread (OS)**:
   - Create shared memory and synchronization tools
   - Create n user threads
   - Process resource requests from user threads
   - Implement the banker's algorithm for deadlock avoidance

2. **User Threads**:
   - Read and process their respective input files
   - Send resource requests to the master thread
   - Simulate delays using `usleep()`
   - Terminate after their final request

## Milestone 5: Inter-Thread Communication

1. **Request Handling Protocol**:
   - User thread locks mutex (`rmtx`)
   - User thread writes request to shared memory
   - Both master and user thread synchronize using `REQB` barrier
   - Master thread reads request
   - Master and user thread synchronize using `ACKB` barrier
   - User thread releases mutex

2. **Request Processing**:
   - RELEASE requests: Non-blocking, update system state immediately
   - ADDITIONAL requests: Enqueue request, check if it can be granted, block thread

3. **Conditional Wait Implementation**:
   - Each user thread has its own condition variable (`cv_i`)
   - Thread blocks on conditional wait after submitting ADDITIONAL request
   - Master thread signals condition when request can be granted

## Milestone 6: Banker's Algorithm Implementation

1. **Safety Check Algorithm**:
   - Create a copy of the system state
   - Find a thread that can complete with available resources
   - Simulate completion of that thread and release its resources
   - Repeat until all threads complete or no thread can proceed

2. **Request Evaluation**:
   - Check if resources are available to fulfill the request
   - If deadlock avoidance is enabled, check if granting the request leads to a safe state
   - Grant request only if both conditions are satisfied

3. **Queue Processing**:
   - After handling any request, check the queue from front to back
   - Try to serve pending requests if possible
   - Remove granted requests from the queue and unblock corresponding threads

## Milestone 7: Program Compilation and Execution

1. **Compilation Options**:
   - Implement a flag `_DLAVOID` to toggle deadlock avoidance
   - Create a makefile with targets for both versions

2. **Execution Modes**:
   - `make allow`: Compile and run without deadlock avoidance
   - `make avoid`: Compile and run with deadlock avoidance
   - `make db`: Generate random input

## Milestone 8: Output and Testing

1. **Logging**:
   - Print resource requests, grants, and releases
   - Show queue status and safe state evaluation
   - Use mutex (`pmtx`) to prevent garbled output

2. **Testing Scenarios**:
   - Test with provided input directories (`input1` and `input2`)
   - `input1`: Should not lead to deadlock but may reach unsafe states
   - `input2`: Should lead to deadlock without avoidance, but complete with avoidance enabled

## Milestone 9: Code Organization and Documentation

1. **Code Structure**:
   - Define clear functions for each major component
   - Separate resource management from thread synchronization logic
   - Implement error handling

2. **Documentation**:
   - Add comments explaining critical sections and algorithms
   - Document the synchronization mechanisms
   - Explain the banker's algorithm implementation

Would you like me to elaborate on any particular milestone or provide more specific implementation details for any part of this assignment?