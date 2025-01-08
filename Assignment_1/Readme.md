Certainly! Below is a detailed `README.md` file explaining the assignment, the problem you're solving, the logic of your program, and how to use it.

---

# OS Lab Assignment 1 - Foodule Rebuild System

## Name: Dadi Sasank Kumar  
## Roll: 22CS10020  
## PC NO: 42

### Description

This program simulates a rebuild process for foodules (modules) with dependencies, inspired by the concept of software modules having build dependencies. It reads the dependencies from the file `foodep.txt`, tracks the visited foodules in `done.txt`, and rebuilds the modules as needed. If a module has unvisited dependencies, the program will recursively rebuild those dependencies in a child process using `fork()` and `exec()`.

The program demonstrates the usage of system calls (`fork()`, `exec()`, and `wait()`) and file I/O in C++ for managing module rebuilds in a dependency tree.

---

### Files

- **`foodep.txt`**: Contains the list of foodules and their dependencies in the format:
  ```
  foodule_id: dependency1, dependency2, ...
  ```
- **`done.txt`**: Stores the list of foodules that have been successfully rebuilt (visited). It is updated after each rebuild.

- **`rebuild.cpp`**: The main source code for the rebuild system.

---

### How the Program Works

The program processes foodule dependencies and rebuilds foodules that are not already rebuilt. The general flow of the program is:

1. **Input Parsing**: 
   - The program reads the foodule number (target) from the command-line arguments.
   - If no specific foodule is provided, the root foodule is assumed, and the program starts the rebuild process.

2. **Dependency Management**:
   - The function `getDependencies()` extracts the dependencies of the given foodule from the file `foodep.txt`.
   - The function `readVisited()` reads the foodules that have already been rebuilt (stored in `done.txt`).
   - The function `writeVisited()` writes the updated list of rebuilt foodules to `done.txt`.

3. **Rebuilding Process**:
   - The `rebuild()` function handles the rebuild process for a specific foodule. If a foodule has not been visited before, it forks a child process to rebuild its dependencies.
   - The rebuild process uses `fork()` to create child processes. The child process uses `execl()` to replace its execution with the rebuild process.
   - The parent process waits for the child processes to finish before marking the foodule as visited.

4. **Output**:
   - After successfully rebuilding a foodule, the program prints a message indicating the foodule has been rebuilt and its dependencies.

---

### Compilation and Usage

#### 1. **Compilation**

You can compile the code using `g++` by running the following command in the terminal:

```bash
g++ -o rebuild rebuild.cpp
```

This will generate an executable named `rebuild`.

#### 2. **Running the Program**

To run the program, you can use the following command:

```bash
./rebuild <foodule_id> [child]
```

- `<foodule_id>`: The ID of the foodule you wish to rebuild.
- `[child]` (optional): If you are running a child process (forked during the rebuild), include this argument.

**Example**:
To rebuild foodule 3 (assuming foodules 1 and 2 are dependencies of 3):

```bash
./rebuild 3
```

If you want to rebuild foodule 2 as a child (this would happen after foodule 1 is rebuilt):

```bash
./rebuild 2 child
```

---

### Dependencies

- **C++ Standard Library**:
  - The program uses standard C++ libraries like `iostream`, `fstream`, `vector`, `sstream`, and system headers for process management (`unistd.h`, `sys/wait.h`).

- **System Calls**:
  - `fork()`: Used to create a new child process.
  - `execl()`: Used to replace the current process with the new process for rebuilding the foodule.
  - `wait()`: Ensures the parent process waits for the child to finish before proceeding.

---

### Detailed Code Explanation

#### 1. **`getDependencies()`**:
This function reads the file `foodep.txt`, skips the lines until the specified `foodule` is reached, and extracts its dependencies. The dependencies are stored in a `vector<int>` and returned.

#### 2. **`readVisited()`**:
Reads the file `done.txt` and returns a vector indicating whether a foodule has already been rebuilt (1 for rebuilt, 0 for not rebuilt).

#### 3. **`writeVisited()`**:
Writes the current state of visited foodules to `done.txt`. This is done after each rebuild to ensure that foodules are not rebuilt multiple times.

#### 4. **`rebuild()`**:
This function is responsible for the main rebuild logic. It:
- Reads the total number of foodules from `foodep.txt`.
- If it is the root foodule, it initializes a vector of size `n` (total number of foodules) and sets all entries to 0 (indicating that no foodules have been rebuilt).
- It then retrieves the dependencies of the current foodule and recursively rebuilds any unvisited dependencies.
- After all dependencies are rebuilt, it marks the current foodule as visited and writes this information to `done.txt`.

#### 5. **`main()`**:
The main entry point of the program:
- It checks if enough arguments are provided (`foodule` ID).
- It parses the command-line arguments and calls the `rebuild()` function with the appropriate parameters.

---

### Example Workflow

**Step 1**: Initially, foodules 1, 2, and 3 are in `foodep.txt` with their respective dependencies:

```
3
1: 2, 3
2: 3
3: 
```

**Step 2**: If you run the command:

```bash
./rebuild 1
```

The program will:
- Start by rebuilding foodule 1.
- It will check dependencies (foodule 2 and 3).
- Fork child processes to rebuild foodules 2 and 3.
- After rebuilding, foodule 1 is marked as visited.

**Step 3**: After a successful rebuild, `done.txt` will be updated to:

```
1 1 1
```

---

### Conclusion

This assignment simulates a build system where foodules (modules) with dependencies are rebuilt in an order that respects the dependency hierarchy. Using system calls like `fork()`, `exec()`, and `wait()`, the program efficiently handles the parallel rebuilding of foodules. 

---

### Future Improvements
- Improve error handling for edge cases, such as malformed input files or missing dependencies.
- Enhance the dependency resolution to handle cyclic dependencies.

--- 

### License

This project is part of the OS Lab coursework and is for educational purposes only.