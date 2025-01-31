#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include "boardgen.c"

#define NUM_BLOCKS 9
#define NUM_CELLS 9
#define DEBUG 1

int pipes[NUM_BLOCKS][2];   // Pipes for communication with blocks
int block_pids[NUM_BLOCKS]; // PIDs of block processes
int current_puzzle[9][9];   // Current puzzle state
int solution_puzzle[9][9];  // Solution state

void print_help()
{
    printf("Commands supported:\n\n");
    printf("        h - Print this help message\n");
    printf("        n - Launch a new puzzle\n");
    printf("        p b c d - Place/replace digit d at cell c of block b\n");
    printf("        s - Show solution\n");
    printf("        q - Quit\n\n");
    printf("Block and Cell Numbering Schema:\n");
    printf("Blocks are numbered 0-8, left to right, top to bottom\n");
    printf("Cells within each block are numbered 0-8, left to right, top to bottom\n");

    printf("\nNumbering schema for blocks and cells:\n");
    printf("+---+---+---+\n");
    printf("| 0 | 1 | 2 |\n");
    printf("+---+---+---+\n");
    printf("| 3 | 4 | 5 |\n");
    printf("+---+---+---+\n");
    printf("| 6 | 7 | 8 |\n");
    printf("+---+---+---+\n");

}

void launch_new_puzzle()
{
    newboard(current_puzzle, solution_puzzle);

    // Send new puzzle data to each block
    for (int block = 0; block < NUM_BLOCKS; block++)
    {
        int block_data[NUM_CELLS];
        int start_row = (block / 3) * 3;
        int start_col = (block % 3) * 3;

        // Extract block data
        int idx = 0;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                block_data[idx++] = current_puzzle[start_row + i][start_col + j];
            }
        }

        // Send data to block
        for (int cell = 0; cell < NUM_CELLS; cell++)
        {
            char cmd[20];
            snprintf(cmd, sizeof(cmd), "n %d %d\n", cell, block_data[cell]);
            write(pipes[block][1], cmd, strlen(cmd));
        }
    }
    
}

void place_digit(int block, int cell, int digit)
{
    if (block < 0 || block >= NUM_BLOCKS ||
        cell < 0 || cell >= NUM_CELLS ||
        digit < 1 || digit > 9)
    {
        printf("Error: Invalid input values\n");
        return;
    }

    char cmd[20];
    snprintf(cmd, sizeof(cmd), "p %d %d\n", cell, digit);
    write(pipes[block][1], cmd, strlen(cmd));
}

void show_solution()
{
    for (int block = 0; block < NUM_BLOCKS; block++)
    {
        int block_data[NUM_CELLS];
        int start_row = (block / 3) * 3;
        int start_col = (block % 3) * 3;

        // Extract solution block data
        int idx = 0;
        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 3; j++)
            {
                block_data[idx++] = solution_puzzle[start_row + i][start_col + j];
            }
        }

        // Send solution to block
        for (int cell = 0; cell < NUM_CELLS; cell++)
        {
            char cmd[20];
            snprintf(cmd, sizeof(cmd), "n %d %d\n", cell, block_data[cell]);
            write(pipes[block][1], cmd, strlen(cmd));
        }
    }
}

void quit()
{
    // Send quit command to all blocks
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        char cmd[] = "q\n";
        write(pipes[i][1], cmd, strlen(cmd));
    }

    // Wait for all blocks to exit
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        waitpid(block_pids[i], NULL, 0);
    }

    exit(0);
}

int main()
{
    // Create pipes for all blocks
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        if (pipe(pipes[i]) == -1)
        {
            perror("pipe creation failed");
            exit(1);
        }
    }

    // Create block processes
    for (int i = 0; i < NUM_BLOCKS; i++)
    {
        pid_t pid = fork();

        if (pid == -1)
        {
            perror("fork failed");
            exit(1);
        }

        if (pid == 0)
        { // Child process (block)
            // Close unused pipes
            for (int j = 0; j < NUM_BLOCKS; j++)
            {
                if (j != i)
                {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
                else
                {
                    close(pipes[j][1]);
                }
            }

            // Prepare file descriptors for the block
            char blockno_str[3], in_fd_str[3], out_fd_str[3];
            char rn1_fd_str[3], rn2_fd_str[3], rn3_fd_str[3], rn0_fd_str[3];
            char cn1_fd_str[3], cn2_fd_str[3], cn3_fd_str[3], cn0_fd_str[3];

            snprintf(blockno_str, sizeof(blockno_str), "%d", i);
            snprintf(in_fd_str, sizeof(in_fd_str), "%d", pipes[i][0]);
            snprintf(out_fd_str, sizeof(out_fd_str), "%d", pipes[i][1]);

            // Set up neighbor pipe descriptors
            // Row neighbors
            if (i % 3 > 0)
                snprintf(rn1_fd_str, sizeof(rn1_fd_str), "%d", pipes[i - 1][1]);
            else
                snprintf(rn1_fd_str, sizeof(rn1_fd_str), "%d", -1);

            if (i % 3 < 2)
                snprintf(rn2_fd_str, sizeof(rn2_fd_str), "%d", pipes[i + 1][1]);
            else
                snprintf(rn2_fd_str, sizeof(rn2_fd_str), "%d", -1);

            // Second row neighbors
            if (i % 3 < 1)
                snprintf(rn3_fd_str, sizeof(rn3_fd_str), "%d", pipes[i + 2][1]);
            else
                snprintf(rn3_fd_str, sizeof(rn3_fd_str), "%d", -1);

            if (i % 3 > 1)
                snprintf(rn0_fd_str, sizeof(rn0_fd_str), "%d", pipes[i - 2][1]);
            else
                snprintf(rn0_fd_str, sizeof(rn0_fd_str), "%d", -1);

            // Column neighbors
            if (i / 3 > 0)
                snprintf(cn1_fd_str, sizeof(cn1_fd_str), "%d", pipes[i - 3][1]);
            else
                snprintf(cn1_fd_str, sizeof(cn1_fd_str), "%d", -1);

            if (i / 3 < 2)
                snprintf(cn2_fd_str, sizeof(cn2_fd_str), "%d", pipes[i + 3][1]);
            else
                snprintf(cn2_fd_str, sizeof(cn2_fd_str), "%d", -1);

            // Second column neighbors
            if (i / 3 < 1)
                snprintf(cn3_fd_str, sizeof(cn3_fd_str), "%d", pipes[i + 6][1]);
            else
                snprintf(cn3_fd_str, sizeof(cn3_fd_str), "%d", -1);

            if (i / 3 > 1)
                snprintf(cn0_fd_str, sizeof(cn0_fd_str), "%d", pipes[i - 6][1]);
            else
                snprintf(cn0_fd_str, sizeof(cn0_fd_str), "%d", -1);

            // Calculate window position
            int x = (i % 3) * 250;
            int y = (i / 3) * 300 - 15;
            char geometry[20];
            snprintf(geometry, sizeof(geometry), "20x8+%d+%d", x, y);

            // Launch block process in xterm
            // Define the maximum length for the title
            char title[20];

            // Adjust the block number by adding 1 to 'i' to start numbering from 1
            snprintf(title, sizeof(title), "Block no: %d", i );

            // Execute the xterm with the formatted title and other parameters
            execlp("xterm", "xterm",
                   "-T", title,                                    // Set the terminal title to "Block no: X"
                   "-fa", "Monospace",                             // Set the font to Monospace
                   "-fs", "15",                                    // Set the font size to 15
                   "-geometry", geometry,                          // Set the geometry (size and position) of the terminal
                   "-bg", "#331100",                               // Set the background color
                   "-e", "./block",                                // Execute the 'block' executable
                   blockno_str,                                    // Block number as a string
                   in_fd_str,                                      // Input file descriptor as a string
                   out_fd_str,                                     // Output file descriptor as a string
                   rn1_fd_str, rn2_fd_str, rn3_fd_str, rn0_fd_str, // Neighbor file descriptors
                   cn1_fd_str, cn2_fd_str, cn3_fd_str, cn0_fd_str, // More neighbor file descriptors
                   NULL);                                          // Null-terminate the argument list

            perror("execlp failed");
            exit(1);
        }
        else
        { // Parent process
            block_pids[i] = pid;
            close(pipes[i][0]);
        }
    }

    // Main command loop
    char cmd;
    print_help();
    while (1)
    {
        printf("Foodoku> ");
        scanf(" %c", &cmd);

        switch (cmd)
        {
        case 'h':
            print_help();
            break;
        case 'n':
            launch_new_puzzle();
            break;
        case 'p':
        {
            int b, c, d;
            scanf("%d %d %d", &b, &c, &d);
            place_digit(b, c, d);
            break;
        }
        case 's':
            show_solution();
            break;
        case 'q':
            quit();
            break;
        default:
            printf("Unknown command. Type 'h' for help.\n");
        }
    }

    return 0;
}