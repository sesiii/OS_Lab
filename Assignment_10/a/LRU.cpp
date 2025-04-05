#include <iostream>
#include <fstream>
#include <cstdint>
#include <queue>
#include <list>
#include <climits>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

using namespace std;

const int PAGE_SIZE = 4096;
const int PAGE_TABLE_ENTRIES = 2048;
const int OS_RESERVED_FRAMES = 4096;
const int TOTAL_FRAMES = 16384;
const int USER_FRAMES = TOTAL_FRAMES - OS_RESERVED_FRAMES;
const int ESSENTIAL_PAGES = 10;
const int NFFMIN = 1000;

struct FrameInfo {
    int frame_number;
    int last_owner_pid;
    int last_owner_page;
};

struct Process {
    int pid;
    int array_size;
    int search_count;
    vector<int> search_keys;
    int current_search;
    vector<uint16_t> page_table;
    vector<uint16_t> page_history;
    int allocated_frames;
    
    // For statistics
    int page_accesses = 0;
    int page_faults = 0;
    int page_replacements = 0;
    vector<int> attempt_counts = {0, 0, 0, 0};
    int last_frame_allocated = -1;
};

class MemoryManager {
private:
    vector<FrameInfo> free_frames_list;
    int nff; // Number of free frames
    queue<Process*> ready_queue;

    int total_page_accesses = 0;
    int total_page_faults = 0;
    int total_page_replacements = 0;
    vector<int> total_attempt_counts = {0, 0, 0, 0};

    void allocateEssentialPages(Process* process) {
        for (int j = 0; j < ESSENTIAL_PAGES; j++) {
            if (nff <= 0) {
                fprintf(stderr, "Not enough free frames during initialization\n");
                exit(1);
            }
            
            // Find a free frame
            auto it = find_if(free_frames_list.begin(), free_frames_list.end(), 
                [](const FrameInfo& fi) { return fi.last_owner_pid == -1; });
                
            if (it == free_frames_list.end()) {
                fprintf(stderr, "No free frame available\n");
                exit(1);
            }
            
            int frame = it->frame_number;
            process->page_table[j] = 0xC000 | frame; // Valid + reference bit set
            process->page_history[j] = 0xFFFF; // Maximum history for essential pages
            
            // Remove from free list
            free_frames_list.erase(it);
            nff--;
            process->allocated_frames++;
        }
    }

    void freeProcessFrames(Process* process) {
        for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            if (process->page_table[i] & 0x8000) { // If valid
                int frame = process->page_table[i] & 0x3FFF; // Get frame number (14 bits)
                
                // Add to free list with no owner
                free_frames_list.push_back({frame, -1, -1});
                nff++;
                
                process->page_table[i] = 0;
                process->page_history[i] = 0;
                process->allocated_frames--;
            }
        }
    }

    vector<FrameInfo>::iterator find_proc(vector<FrameInfo>::iterator begin1,vector<FrameInfo>::iterator end1){
        for(auto i = begin1;i!=end1;i++){
            if(i->last_owner_pid==-1){
                return i;
            }
        }
        return end1;
    }

    bool handlePageFault(Process* process, int page_num) {
        process->page_faults++;
        total_page_faults++;
        
        if (nff > NFFMIN) {
            // Case 1: Enough free frames, just allocate one
            auto it = find_if(free_frames_list.begin(), free_frames_list.end(), 
                [](const FrameInfo& fi) { return fi.last_owner_pid == -1; });
                
            if (it == free_frames_list.end()) {
                fprintf(stderr, "No free frame available despite nff > NFFMIN\n");
                exit(1);
            }
            
            int frame = it->frame_number;
            process->page_table[page_num] = 0xC000 | frame; // Valid + reference bit set
            process->page_history[page_num] = 0xFFFF;
            process->last_frame_allocated = frame;
            
            // Remove from free list
            free_frames_list.erase(it);
            nff--;
            process->allocated_frames++;
            return true;
        } else {
            
            // Case 2: Need to perform page replacement
            process->page_replacements++;
            total_page_replacements++;
            
            // Find victim page with minimum history (excluding essential pages)
            uint16_t min_history = 0xFFFF;
            int victim_page = -1;
            
            for (int i = ESSENTIAL_PAGES; i < PAGE_TABLE_ENTRIES; i++) {
                if ((process->page_table[i] & 0x8000) && // Valid
                    process->page_history[i] < min_history) {
                    min_history = process->page_history[i];
                    victim_page = i;
                }
            }
            
            if (victim_page == -1) {
                fprintf(stderr, "No victim page found for replacement\n");
                exit(1);
            }
            
            // Get the frame of the victim page
            int victim_frame = process->page_table[victim_page] & 0x3FFF;
            //    Fault on Page  345: To replace Page 1345 at Frame 8831 [history = 63]
            #ifdef VERBOSE
                printf("\tFault on Page  %d: To replace Page %d at Frame %d [history = %d]\n",page_num,victim_page,victim_frame,min_history);
            #endif
            
            // Attempt 1: Check if there's a frame in free list with same owner and page
            auto it = find_if(free_frames_list.begin(), free_frames_list.end(), 
                [process, page_num](const FrameInfo& fi) { 
                    return fi.last_owner_pid == process->pid && fi.last_owner_page == page_num; 
                });
            
            if (it != free_frames_list.end()) {
                // Attempt 1 success
                process->attempt_counts[0]++;
                total_attempt_counts[0]++;
                
                
                int frame = it->frame_number;
                #ifdef VERBOSE
                    printf("\t\tAttempt 1: Page found in free frame %d\n",frame);
                #endif
                process->page_table[page_num] = 0xC000 | frame;
                process->page_history[page_num] = 0xFFFF;
                process->last_frame_allocated = frame;
                
                // Update free list - add victim frame with owner info
                free_frames_list.erase(it);
                free_frames_list.push_back({victim_frame, process->pid, victim_page});
            } 
            // Attempt 2: Find any free frame with no owner
            else if ((it = find_proc(free_frames_list.begin(),free_frames_list.end())) != free_frames_list.end()) {
                // Attempt 2 success
                process->attempt_counts[1]++;
                total_attempt_counts[1]++;
                
                int frame = it->frame_number;

                #ifdef VERBOSE
                    printf("\t\tAttempt 2: Free frame %d owned by no process found\n",frame);
                #endif
                fflush(stdout);
                process->page_table[page_num] = 0xC000 | frame;
                process->page_history[page_num] = 0xFFFF;
                process->last_frame_allocated = frame;
                
                // Update free list - add victim frame with owner info
                free_frames_list.erase(it);
                free_frames_list.push_back({victim_frame, process->pid, victim_page});
            } 
            // Attempt 3: Find free frame with same owner
            else if ((it = find_if(free_frames_list.begin(), free_frames_list.end(), 
                [process](const FrameInfo& fi) { return fi.last_owner_pid == process->pid; })) != free_frames_list.end()) {
                // Attempt 3 success
                process->attempt_counts[2]++;
                total_attempt_counts[2]++;
                
                int frame = it->frame_number;

                #ifdef VERBOSE
                    printf("\t\tAttempt 3: Own page %d found in free frame %d\n",page_num,frame);
                #endif
            
                process->page_table[page_num] = 0xC000 | frame;
                process->page_history[page_num] = 0xFFFF;
                process->last_frame_allocated = frame;
                
                // Update free list - add victim frame with owner info
                free_frames_list.erase(it);
                free_frames_list.push_back({victim_frame, process->pid, victim_page});
            } 
            // Attempt 4: Just pick any free frame
            else if (!free_frames_list.empty()) {
                // Attempt 4 success
                process->attempt_counts[3]++;
                total_attempt_counts[3]++;
                
                int frame = free_frames_list.front().frame_number;
                process->page_table[page_num] = 0xC000 | frame;
                process->page_history[page_num] = 0xFFFF;
                process->last_frame_allocated = frame;
                #ifdef VERBOSE
                    printf("\t\tAttempt 4: Free frame %d owned by Process %d chosen\n",frame,free_frames_list.front().last_owner_pid );
                #endif
                
                // Update free list - add victim frame with owner info
                free_frames_list.erase(free_frames_list.begin());
                free_frames_list.push_back({victim_frame, process->pid, victim_page});
            } else {
                fprintf(stderr, "No free frame available for replacement\n");
                exit(1);
            }
            
            // Invalidate victim page
            process->page_table[victim_page] &= 0x3FFF; // Clear valid bit
            process->page_history[victim_page] = 0;
            process->allocated_frames--;
        }
        return false;
    }

    void updatePageHistories(Process* process) {
        for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
            if (process->page_table[i] & 0x8000) { // If valid
                // Right shift history and insert reference bit at MSB
                uint16_t ref_bit = (process->page_table[i] & 0x4000) ? 1 : 0;
                process->page_history[i] = (process->page_history[i] >> 1) | (ref_bit << 15);
                
                // Clear reference bit
                process->page_table[i] &= 0xBFFF;
            }
        }
    }

    void processCompletion(Process* process) {
        freeProcessFrames(process);
    }

    void performBinarySearch(Process* process) {
#ifdef VERBOSE
printf("+++ Process %3d: Search %d\n", process->pid, process->current_search + 1);
#endif
        int key = process->search_keys[process->current_search];
        int left = 0, right = process->array_size - 1;

        while (left < right) {
            int mid = (left + right) / 2;
            int page_num = ESSENTIAL_PAGES + (mid / (PAGE_SIZE / sizeof(int)));

            process->page_accesses++;
            total_page_accesses++;

            // Set reference bit
            process->page_table[page_num] |= 0x4000;

            if ((process->page_table[page_num] & 0x8000) == 0) {
                bool ans = handlePageFault(process, page_num);
                #ifdef VERBOSE
                if(ans)
                    printf("    Fault on Page %4d: Free frame %d found\n", 
                        page_num, process->last_frame_allocated);

                #endif
            }

            if (key <= mid) right = mid;
            else left = mid + 1;
        }

        // Update page histories after search completion
        updatePageHistories(process);

        process->current_search++;

        if (process->current_search == process->search_count) {
            processCompletion(process);
        } else {
            ready_queue.push(process);
        }
    }

public:
    void loadProcesses(const string& filename, vector<Process>& processes) {
        ifstream file(filename);
        if (!file.is_open()) {
            fprintf(stderr, "Error opening file\n");
            exit(1);
        }

        int n, m;
        file >> n >> m;
        processes.resize(n);

        for (int i = 0; i < n; i++) {
            Process& process = processes[i];
            process.pid = i;
            file >> process.array_size;
            process.search_count = m;
            process.search_keys.resize(m);
            for (int j = 0; j < m; j++) {
                file >> process.search_keys[j];
            }
            process.current_search = 0;
            process.page_table.resize(PAGE_TABLE_ENTRIES, 0);
            process.page_history.resize(PAGE_TABLE_ENTRIES, 0);
            process.allocated_frames = 0;

            allocateEssentialPages(&process);
            ready_queue.push(&process);
        }
        file.close();
    }

    void simulate() {
        while (true) {
            if (ready_queue.empty()) {
                break;
            }

            Process* process = ready_queue.front();
            ready_queue.pop();

            if (process->current_search < process->search_count) {
                performBinarySearch(process);
            }
        }
    }

    void printSummary(const vector<Process>& processes) {
        printf("+++ Page access summary\n");
        printf(" PID     Accesses        Faults         Replacements                        Attempts\n");
        
        for (const auto& process : processes) {
            if (process.current_search < process.search_count) continue; // Skip incomplete processes
            
            printf("%4d %10d %6d (%5.2f%%) %6d (%5.2f%%) %6d + %3d + %3d + %3d (%5.2f%% + %4.2f%% + %5.2f%% + %4.2f%%)\n",
                   process.pid,
                   process.page_accesses,
                   process.page_faults,
                   (float)process.page_faults * 100 / process.page_accesses,
                   process.page_replacements,
                   (float)process.page_replacements * 100 / process.page_accesses,
                   process.attempt_counts[0],
                   process.attempt_counts[1],
                   process.attempt_counts[2],
                   process.attempt_counts[3],
                   (float)process.attempt_counts[0] * 100 / max(1, process.page_replacements),
                   (float)process.attempt_counts[1] * 100 / max(1, process.page_replacements),
                   (float)process.attempt_counts[2] * 100 / max(1, process.page_replacements),
                   (float)process.attempt_counts[3] * 100 / max(1, process.page_replacements));
        }
        
        printf("\nTotal %10d %6d (%5.2f%%) %6d (%5.2f%%) %6d + %3d + %5d + %3d (%5.2f%% + %4.2f%% + %5.2f%% + %4.2f%%)\n",
               total_page_accesses,
               total_page_faults,
               (float)total_page_faults * 100 / total_page_accesses,
               total_page_replacements,
               (float)total_page_replacements * 100 / total_page_accesses,
               total_attempt_counts[0],
               total_attempt_counts[1],
               total_attempt_counts[2],
               total_attempt_counts[3],
               (float)total_attempt_counts[0] * 100 / max(1, total_page_replacements),
               (float)total_attempt_counts[1] * 100 / max(1, total_page_replacements),
               (float)total_attempt_counts[2] * 100 / max(1, total_page_replacements),
               (float)total_attempt_counts[3] * 100 / max(1, total_page_replacements));
    }

    MemoryManager() {
        // Initialize free frames list
        for (int i = 0; i < USER_FRAMES; i++) {
            free_frames_list.push_back({OS_RESERVED_FRAMES + i, -1, -1});
        }
        nff = USER_FRAMES;
    }
};

int main(int argc, char* argv[]) {
    char* filename;
    vector<Process> processes;
    #ifdef VERBOSE
        filename = strdup("verboseoutput.txt");
    #else
        filename = strdup("output.txt");
    #endif

    int ofd = open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(ofd < 0){
        perror("FILE FD error\n");
        exit(1);
    }
    close(1);

    dup(ofd);
    close(ofd);
    
    MemoryManager memory_manager;
    memory_manager.loadProcesses("search.txt", processes);
    
    printf("g++ -Wall -DVERBOSE -o runsearch LRU.cpp\n");
    printf("./runsearch\n");
    
    memory_manager.simulate();
    memory_manager.printSummary(processes);

    return 0;
}