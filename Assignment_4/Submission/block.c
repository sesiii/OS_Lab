#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define NUM_CELLS 9
#define DEBUG 1

int A[3][3]; // Original puzzle block
int B[3][3]; // Current state block
int blockno;
int in_fd, out_fd;
int rn1_fd, rn2_fd, rn3_fd, rn0_fd; // Row neighbor file descriptors
int cn1_fd, cn2_fd, cn3_fd, cn0_fd; // Column neighbor file descriptors

void draw_block()
{
 
    // printf("Block %d:\n", blockno);
    printf("\n\n");
    printf("+---+---+---+\n");
    for (int i = 0; i < 3; i++)
    {
        printf("|");
        for (int j = 0; j < 3; j++)
        {
            if (B[i][j] == 0)
                printf(" . |");
            else
                printf(" %d |", B[i][j]);
        }
        printf("\n+---+---+---+\n");
    }
    
    fflush(stdout);
}


int check_row_conflict(int local_row, int digit)
{
    for (int j = 0; j < 3; j++)
    {
        if (B[local_row][j] == digit)
            return 1;
    }

    int global_row = (blockno / 3) * 3 + local_row;
    char cmd[20], response[10];

    // Check left neighbor
    if (rn1_fd != -1)
    {
        snprintf(cmd, sizeof(cmd), "r %d %d\n", global_row, digit);
        if (write(rn1_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(rn1_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
    }
    else if (rn2_fd != -1) // Check two right neighbors if no left neighbor
    {
        snprintf(cmd, sizeof(cmd), "r %d %d\n", global_row, digit);
        if (write(rn2_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(rn2_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
        // Assuming rn3_fd is the second right neighbor
        if (rn3_fd != -1)
        {
            if (write(rn3_fd, cmd, strlen(cmd)) > 0)
            {
                memset(response, 0, sizeof(response));
                if (read(rn3_fd, response, sizeof(response) - 1) > 0)
                {
                    if (response[0] == '1')
                        return 1;
                }
            }
        }
    }

    // Check right neighbor
    if (rn2_fd != -1)
    {
        snprintf(cmd, sizeof(cmd), "r %d %d\n", global_row, digit);
        if (write(rn2_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(rn2_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
    }
    else if (rn1_fd != -1) // Check two left neighbors if no right neighbor
    {
        snprintf(cmd, sizeof(cmd), "r %d %d\n", global_row, digit);
        if (write(rn1_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(rn1_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
        // Assuming rn0_fd is the second left neighbor
        if (rn0_fd != -1)
        {
            if (write(rn0_fd, cmd, strlen(cmd)) > 0)
            {
                memset(response, 0, sizeof(response));
                if (read(rn0_fd, response, sizeof(response) - 1) > 0)
                {
                    if (response[0] == '1')
                        return 1;
                }
            }
        }
    }

    return 0; // Default: No conflict
}

int check_column_conflict(int local_col, int digit)
{
    for (int i = 0; i < 3; i++)
    {
        if (B[i][local_col] == digit)
            return 1;
    }

    int global_col = (blockno % 3) * 3 + local_col;
    char cmd[20], response[10];

    // Check upper neighbor
    if (cn1_fd != -1)
    {
        snprintf(cmd, sizeof(cmd), "c %d %d\n", global_col, digit);
        if (write(cn1_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(cn1_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
    }
    else if (cn2_fd != -1) // Check two lower neighbors if no upper neighbor
    {
        snprintf(cmd, sizeof(cmd), "c %d %d\n", global_col, digit);
        if (write(cn2_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(cn2_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
        // Assuming cn3_fd is the second lower neighbor
        if (cn3_fd != -1)
        {
            if (write(cn3_fd, cmd, strlen(cmd)) > 0)
            {
                memset(response, 0, sizeof(response));
                if (read(cn3_fd, response, sizeof(response) - 1) > 0)
                {
                    if (response[0] == '1')
                        return 1;
                }
            }
        }
    }

    // Check lower neighbor
    if (cn2_fd != -1)
    {
        snprintf(cmd, sizeof(cmd), "c %d %d\n", global_col, digit);
        if (write(cn2_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(cn2_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
    }
    else if (cn1_fd != -1) // Check two upper neighbors if no lower neighbor
    {
        snprintf(cmd, sizeof(cmd), "c %d %d\n", global_col, digit);
        if (write(cn1_fd, cmd, strlen(cmd)) > 0)
        {
            memset(response, 0, sizeof(response));
            if (read(cn1_fd, response, sizeof(response) - 1) > 0)
            {
                if (response[0] == '1')
                    return 1;
            }
        }
        // Assuming cn0_fd is the second upper neighbor
        if (cn0_fd != -1)
        {
            if (write(cn0_fd, cmd, strlen(cmd)) > 0)
            {
                memset(response, 0, sizeof(response));
                if (read(cn0_fd, response, sizeof(response) - 1) > 0)
                {
                    if (response[0] == '1')
                        return 1;
                }
            }
        }
    }

    return 0; // Default: No conflict
}

void handle_place(int cell, int digit)
{
    int local_row = cell / 3;
    int local_col = cell % 3;

    // Check if cell is read-only
    if (A[local_row][local_col] != 0)
    {
        printf("Error: Read-only cell\n");
        fflush(stdout);
        sleep(2);
        draw_block();
        return;
    }

    // Check block conflict
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (B[i][j] == digit)
            {
                printf("Error: Block conflict\n");
                fflush(stdout);
                sleep(2);
                draw_block();
                return;
            }
        }
    }

    // Check row conflict
    if (check_row_conflict(local_row, digit))
    {
        printf("Error: Row conflict\n");
        fflush(stdout);
        sleep(2);
        draw_block();
        return;
    }

    // Check column conflict
    if (check_column_conflict(local_col, digit))
    {
        printf("Error: Column conflict\n");
        fflush(stdout);
        sleep(2);
        draw_block();
        return;
    }

    // If no conflicts, place the digit
    B[local_row][local_col] = digit;
    draw_block();
}

void handle_command(char cmd, int c, int d)
{
    // if (DEBUG)
        // fprintf(stderr, "[Block %d] Received command: %c %d %d\n", blockno, cmd, c, d);
    // draw_block();
    switch (cmd)
    {
    case 'n':
    { // New value
        int row = c / 3;
        int col = c % 3;
        A[row][col] = d;
        B[row][col] = d;
        draw_block();
        break;
    }
    case 'p': // Place digit
        handle_place(c, d);
        break;

    case 'r':
    { // Row check request
        int local_row = c % 3;
        int conflict = 0;

        // Check the specified row in current block
        for (int j = 0; j < 3; j++)
        {
            if (B[local_row][j] == d)
            {
                conflict = 1;
                break;
            }
        }

        // Send response
        char response[10];
        snprintf(response, sizeof(response), "%d\n", conflict);
        write(out_fd, response, strlen(response));
        if (DEBUG)
            fprintf(stderr, "[Block %d] Sent row check response: %s", blockno, response);
        break;
    }

    case 'c':
    { // Column check request
        int local_col = c % 3;
        int conflict = 0;

        // Check the specified column in current block
        for (int i = 0; i < 3; i++)
        {
            if (B[i][local_col] == d)
            {
                conflict = 1;
                break;
            }
        }

        // Send response
        char response[10];
        snprintf(response, sizeof(response), "%d\n", conflict);
        write(out_fd, response, strlen(response));
        
        break;
    }

    case 'q': // Quit
        printf("Bye...\n");
        fflush(stdout);
        sleep(1);
        exit(0);

    default:
        printf("Unknown command\n");
        fflush(stdout);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 12)
    {
        fprintf(stderr, "Usage: %s blockno in_fd out_fd rn1_fd rn2_fd rn3_fd rn0_fd cn1_fd cn2_fd cn3_fd cn0_fd\n", argv[0]);
        exit(1);
    }
    
    // Parse command line arguments
    blockno = atoi(argv[1]);
    in_fd = atoi(argv[2]);
    out_fd = atoi(argv[3]);
    rn1_fd = atoi(argv[4]);
    rn2_fd = atoi(argv[5]);
    rn3_fd = atoi(argv[6]);
    rn0_fd = atoi(argv[7]);
    cn1_fd = atoi(argv[8]);
    cn2_fd = atoi(argv[9]);
    cn3_fd = atoi(argv[10]);
    cn0_fd = atoi(argv[11]);

    if (DEBUG)
    {
        fprintf(stderr, "[Block %d] Ready\n", blockno);
        // fprintf(stderr, "  in_fd=%d, out_fd=%d\n", in_fd, out_fd);
        // fprintf(stderr, "  rn1_fd=%d, rn2_fd=%d, rn3_fd=%d, rn0_fd=%d\n", rn1_fd, rn2_fd, rn3_fd, rn0_fd);
        // fprintf(stderr, "  cn1_fd=%d, cn2_fd=%d, cn3_fd=%d, cn0_fd=%d\n", cn1_fd, cn2_fd, cn3_fd, cn0_fd);
    }

    // Initialize block arrays
    memset(A, 0, sizeof(A));
    memset(B, 0, sizeof(B));

    // Set up standard input
    close(STDIN_FILENO);
    dup2(in_fd, STDIN_FILENO);
    close(in_fd);

    // Main command processing loop
    char cmd_line[100];
    while (fgets(cmd_line, sizeof(cmd_line), stdin))
    {
        char cmd;
        int arg1, arg2;

        // Parse command line
        if (sscanf(cmd_line, " %c %d %d", &cmd, &arg1, &arg2) >= 1)
        {
            handle_command(cmd, arg1, arg2);
        }
    }

    return 0;
}