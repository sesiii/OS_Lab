#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define TOTAL_MEMORY_FRAMES 16384
#define USER_MEMORY_FRAMES 12288
#define ESSENTIAL_SEGMENT_PAGES 10
#define PAGE_SIZE_BYTES 4096
#define INTEGERS_PER_PAGE 1024
#define VIRTUAL_PAGES_PER_PROCESS 2048
#define MAX_PROCESS_COUNT 500
#define MAX_SEARCH_COUNT 100
#define NFFMIN 1000

typedef struct
{
    int process_id;
    int array_size;
    int current_search_number;
    int search_keys[MAX_SEARCH_COUNT];
} SystemProcess;

typedef struct
{
    unsigned short frame_number;
    unsigned short valid_ref;
    unsigned short history;
} PageTableEntry;

typedef struct
{
    int frame_number;
    int last_owner;
    int last_page_number;
} FreeFrameEntry;

SystemProcess *system_processes;
PageTableEntry *process_page_tables[MAX_PROCESS_COUNT];
FreeFrameEntry *fflist;
int total_processes, total_searches;
int nff = USER_MEMORY_FRAMES;
int total_page_accesses = 0;
int total_page_faults = 0;
int total_replacements = 0;
int attempt_counts[4] = {0, 0, 0, 0};

typedef struct
{
    int accesses;
    int faults;
    int replacements;
    int attempts[4];
} ProcessStats;

ProcessStats *process_stats;

void initialize_fflist()
{
    fflist = (FreeFrameEntry *)malloc(USER_MEMORY_FRAMES * sizeof(FreeFrameEntry));
    for (int i = 0; i < USER_MEMORY_FRAMES; i++)
    {
        fflist[i].frame_number = i;
        fflist[i].last_owner = -1;
        fflist[i].last_page_number = -1;
    }
}

int find_victim_page(int proc_id)
{
    unsigned short min_history = 0xFFFF;
    int victim_page = -1;

    for (int i = ESSENTIAL_SEGMENT_PAGES; i < VIRTUAL_PAGES_PER_PROCESS; i++)
    {
        if (process_page_tables[proc_id][i].valid_ref & (1 << 15))
        { // Valid bit check
            if (process_page_tables[proc_id][i].history < min_history)
            {
                min_history = process_page_tables[proc_id][i].history;
                victim_page = i;
            }
        }
    }
    return victim_page;
}

int allocate_frame(int proc_id, int page_number)
{
    total_page_faults++;
    process_stats[proc_id].faults++;

    int frame_number;

    if (nff > NFFMIN)
    {
        nff--;
        int frame_idx = nff;
        frame_number = fflist[frame_idx].frame_number;
#ifdef VERBOSE
        printf("    Fault on Page %4d: Free frame %d found\n", page_number, frame_number);
#endif
    }
    else
    {
        // Page replacement needed
        total_replacements++;
        process_stats[proc_id].replacements++;

        int frame_idx = -1;
        int attempt = 0;
        int victim_page = find_victim_page(proc_id);
        int victim_frame = process_page_tables[proc_id][victim_page].frame_number;
        unsigned short victim_history = process_page_tables[proc_id][victim_page].history;

        // Attempt 1: Same process, same page
        for (int i = 0; i < nff; i++)
        {
            if (fflist[i].last_owner == proc_id && fflist[i].last_page_number == page_number)
            {
                frame_idx = i;
                attempt = 1;
                attempt_counts[0]++;
                process_stats[proc_id].attempts[0]++;
                break;
            }
        }

        // Attempt 2: No owner
        if (frame_idx == -1)
        {
            for (int i = 0; i < nff; i++)
            {
                if (fflist[i].last_owner == -1)
                {
                    frame_idx = i;
                    attempt = 2;
                    attempt_counts[1]++;
                    process_stats[proc_id].attempts[1]++;
                    break;
                }
            }
        }

        // Attempt 3: Same process, different page
        if (frame_idx == -1)
        {
            for (int i = 0; i < nff; i++)
            {
                if (fflist[i].last_owner == proc_id)
                {
                    frame_idx = i;
                    attempt = 3;
                    attempt_counts[2]++;
                    process_stats[proc_id].attempts[2]++;
                    break;
                }
            }
        }

        // Attempt 4: Random frame
        if (frame_idx == -1)
        {
            frame_idx = rand() % nff;
            attempt = 4;
            attempt_counts[3]++;
            process_stats[proc_id].attempts[3]++;
        }

        frame_number = fflist[frame_idx].frame_number;

#ifdef VERBOSE
        printf("    Fault on Page %4d: To replace Page %4d at Frame %d [history = %d]\n", page_number, victim_page, victim_frame, victim_history);
        if (attempt == 1)
        {
            printf("        Attempt 1: Same page %d found in free frame %d\n", page_number, frame_number);
        }
        else if (attempt == 2)
        {
            printf("        Attempt 2: Free frame %d found with no owner\n", frame_number);
        }
        else if (attempt == 3)
        {
            printf("        Attempt 3: Own page %d found in free frame %d\n", fflist[frame_idx].last_page_number, frame_number);
        }
        else
        {
            printf("        Attempt 4: Random free frame %d found\n", frame_number);
        }
#endif

        // Update FFLIST with victim page info
        fflist[frame_idx].frame_number = victim_frame;
        fflist[frame_idx].last_owner = proc_id;
        fflist[frame_idx].last_page_number = victim_page;

        process_page_tables[proc_id][victim_page].valid_ref &= ~(1 << 15);
    }

    process_page_tables[proc_id][page_number].frame_number = frame_number;
    process_page_tables[proc_id][page_number].valid_ref |= (1 << 15);
    process_page_tables[proc_id][page_number].history = 0xFFFF;

    return frame_number;
}
void update_history(int proc_id)
{
    for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++)
    {
        if (process_page_tables[proc_id][i].valid_ref & (1 << 15))
        {
            unsigned short ref_bit = (process_page_tables[proc_id][i].valid_ref >> 14) & 1;
            process_page_tables[proc_id][i].history = (process_page_tables[proc_id][i].history >> 1) | (ref_bit << 15);
            process_page_tables[proc_id][i].valid_ref &= ~(1 << 14);
        }
    }
}

void free_process_frames(int proc_id)
{
    for (int i = 0; i < VIRTUAL_PAGES_PER_PROCESS; i++)
    {
        if (process_page_tables[proc_id][i].valid_ref & (1 << 15))
        {
            fflist[nff].frame_number = process_page_tables[proc_id][i].frame_number;
            fflist[nff].last_owner = -1;
            fflist[nff].last_page_number = -1;
            process_page_tables[proc_id][i].valid_ref &= ~(1 << 15);
            nff++;
        }
    }
}

int main()
{
    FILE *input_file = fopen("search.txt", "r");
    if (!input_file)
    {
        printf("Could not open search.txt\n");
        return 1;
    }

    fscanf(input_file, "%d %d", &total_processes, &total_searches);

    system_processes = (SystemProcess *)malloc(total_processes * sizeof(SystemProcess));
    process_stats = (ProcessStats *)calloc(total_processes, sizeof(ProcessStats));

    for (int i = 0; i < total_processes; i++)
    {
        process_page_tables[i] = (PageTableEntry *)calloc(VIRTUAL_PAGES_PER_PROCESS, sizeof(PageTableEntry));
    }

    initialize_fflist();

    for (int i = 0; i < total_processes; i++)
    {
        fscanf(input_file, "%d", &system_processes[i].array_size);
        system_processes[i].process_id = i;
        system_processes[i].current_search_number = 0;

        for (int j = 0; j < total_searches; j++)
        {
            fscanf(input_file, "%d", &system_processes[i].search_keys[j]);
        }

        for (int j = 0; j < ESSENTIAL_SEGMENT_PAGES; j++)
        {
            nff--;
            process_page_tables[i][j].frame_number = fflist[nff].frame_number;
            process_page_tables[i][j].valid_ref |= (1 << 15);
            process_page_tables[i][j].history = 0xFFFF;
        }
    }
    fclose(input_file);

    int active_processes = total_processes;
    while (active_processes > 0)
    {
        for (int i = 0; i < total_processes && active_processes > 0; i++)
        {
            if (system_processes[i].current_search_number >= total_searches)
                continue;
#ifdef VERBOSE
            printf("+++ Process %d: Search %d\n", i, system_processes[i].current_search_number + 1);
#endif
            int search_key = system_processes[i].search_keys[system_processes[i].current_search_number];
            int left = 0, right = system_processes[i].array_size - 1;

            // Binary search
            while (left < right)
            {
                int mid = (left + right) / 2;
                int page_index = ESSENTIAL_SEGMENT_PAGES + (mid / INTEGERS_PER_PAGE);

                total_page_accesses++;
                process_stats[i].accesses++;

                if (!(process_page_tables[i][page_index].valid_ref & (1 << 15)))
                {
                    allocate_frame(i, page_index);
                }
                else
                {
#ifdef VERBOSE
                    // printf("    Access Page %4d: In frame %d\n",page_index, process_page_tables[i][page_index].frame_number);
#endif
                }

                process_page_tables[i][page_index].valid_ref |= (1 << 14);

                if (search_key <= mid)
                    right = mid;
                else
                    left = mid + 1;
            }

            update_history(i);
            system_processes[i].current_search_number++;

            if (system_processes[i].current_search_number >= total_searches)
            {
#ifdef VERBOSE
                printf("    Process %d completed, freeing frames\n", i);
#endif
                free_process_frames(i);
                active_processes--;
            }
        }
    }

    // statistics
    printf("+++ Page access summary\n");
    printf("PID     Accesses         Faults             Replacements                             Attempts\n");

    for (int i = 0; i < total_processes; i++)
    {
        float fault_percent = (float)process_stats[i].faults * 100 / process_stats[i].accesses;
        float replace_percent = (float)process_stats[i].replacements * 100 / process_stats[i].accesses;
        float a1 = process_stats[i].replacements ? (float)process_stats[i].attempts[0] * 100 / process_stats[i].replacements : 0;
        float a2 = process_stats[i].replacements ? (float)process_stats[i].attempts[1] * 100 / process_stats[i].replacements : 0;
        float a3 = process_stats[i].replacements ? (float)process_stats[i].attempts[2] * 100 / process_stats[i].replacements : 0;
        float a4 = process_stats[i].replacements ? (float)process_stats[i].attempts[3] * 100 / process_stats[i].replacements : 0;

        printf("%d        %d         %d    (%.2f%%)     %d   (%.2f%%)         %d + %d + %d + %d (%.2f%% + %.2f%% + %.2f%% + %.2f%%)\n",
               i, process_stats[i].accesses, process_stats[i].faults, fault_percent,
               process_stats[i].replacements, replace_percent,
               process_stats[i].attempts[0], process_stats[i].attempts[1],
               process_stats[i].attempts[2], process_stats[i].attempts[3],
               a1, a2, a3, a4);
    }

    float total_fault_percent = (float)total_page_faults * 100 / total_page_accesses;
    float total_replace_percent = (float)total_replacements * 100 / total_page_accesses;
    float total_a1 = total_replacements ? (float)attempt_counts[0] * 100 / total_replacements : 0;
    float total_a2 = total_replacements ? (float)attempt_counts[1] * 100 / total_replacements : 0;
    float total_a3 = total_replacements ? (float)attempt_counts[2] * 100 / total_replacements : 0;
    float total_a4 = total_replacements ? (float)attempt_counts[3] * 100 / total_replacements : 0;

    printf("\n");
    printf("Total     %d       %d   (%.2f%%)     %d (%.2f%%)         %d + %d + %d + %d (%.2f%% + %.2f%% + %.2f%% + %.2f%%)\n", total_page_accesses, total_page_faults, total_fault_percent, total_replacements, total_replace_percent, attempt_counts[0], attempt_counts[1], attempt_counts[2], attempt_counts[3], total_a1, total_a2, total_a3, total_a4);

    free(system_processes);
    free(fflist);
    free(process_stats);
    for (int i = 0; i < total_processes; i++)
    {
        free(process_page_tables[i]);
    }

    return 0;
}