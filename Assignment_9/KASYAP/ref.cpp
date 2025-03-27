#include <bits/stdc++.h>
using namespace std;

#define TOTAL_FRAMES 16384
#define USER_FRAMES 12288
#define ESSENTIAL_PAGES 10
#define PAGE_SIZE 4096

int n, m;
int tot_page_accs = 0;
int tot_page_faults = 0;
int tot_swaps = 0;
int available_frames = USER_FRAMES;
int multi_prog = INT_MAX;
int completed_count = 0;

typedef struct {
    int pid;
    int addi_data_seg;
    int cur_search_idx;
    int max_A_size;
    vector<int> searches;
    bool is_swapped;
} _process;

_process *process;

typedef struct {
    uint16_t entries[2048];
} _page_table;

_page_table *page_tables;

queue<_process *> ready_queue;
queue<_process *> swapped_out;

void page_set(_page_table *page_tables, int process_id, int entry) {
    if (!(page_tables[process_id].entries[entry] & (1 << 15))) {
        page_tables[process_id].entries[entry] |= (1 << 15);
        if (entry >= ESSENTIAL_PAGES) process[process_id].addi_data_seg++;
        available_frames--;
    }
}

void page_clear(_page_table *page_tables, int process_id, int entry) {
    if (page_tables[process_id].entries[entry] & (1 << 15)) {
        page_tables[process_id].entries[entry] &= ~(1 << 15);
        if (entry >= ESSENTIAL_PAGES) process[process_id].addi_data_seg--;
        available_frames++;
    }
}

bool page_retrieve(_page_table *page_tables, int process_id, int entry) {
    return page_tables[process_id].entries[entry] >> 15;
}

void swap_out(_process *proc) {
    proc->is_swapped = true;
    for (int i = 0; i < 2048; i++) {
        if (page_retrieve(page_tables, proc->pid, i)) {
            page_clear(page_tables, proc->pid, i);
        }
    }
    proc->addi_data_seg = 0;
    swapped_out.push(proc);
    
    int swapped_out_count = swapped_out.size();
    int active = n - swapped_out_count - completed_count;
    printf("+++ Swapping out process %3d [%d active processes]\n", proc->pid, active);
    tot_swaps++;
    multi_prog = min(multi_prog, active);
}

void swap_in(_process *proc) {
    proc->is_swapped = false;
    for (int i = 0; i < ESSENTIAL_PAGES; i++) {
        page_set(page_tables, proc->pid, i);
    }
    
    deque<_process *> temp;
    temp.push_front(proc);
    while (!ready_queue.empty()) {
        temp.push_back(ready_queue.front());
        ready_queue.pop();
    }
    while (!temp.empty()) {
        ready_queue.push(temp.front());
        temp.pop_front();
    }
    
    int swapped_out_count = swapped_out.size();
    int active = n - swapped_out_count - completed_count;
    printf("+++ Swapping in process %3d [%d active processes]\n", proc->pid, active);
}

int main() {
    ifstream fin("search.txt");
    if (!fin) {
        printf("Error: Could not open search.txt\n");
        return 1;
    }
    
    fin >> n >> m;
    page_tables = (_page_table *)malloc(n * sizeof(_page_table));
    process = (_process *)malloc(n * sizeof(_process));
    
    printf("+++ Simulation data read from file\n");
    printf("+++ Kernel data initialized\n");

    for (int i = 0; i < n; i++) {
        fin >> process[i].max_A_size;
        process[i].searches.resize(m);
        process[i].is_swapped = false;
        process[i].cur_search_idx = 0;
        process[i].addi_data_seg = 0;
        process[i].pid = i;
        for (int j = 0; j < ESSENTIAL_PAGES; j++) {
            page_set(page_tables, i, j);
        }
        for (int j = 0; j < m; j++) {
            fin >> process[i].searches[j];
        }
        ready_queue.push(&process[i]);
    }
    fin.close();
    multi_prog = n;

    while (!ready_queue.empty() || !swapped_out.empty()) {
        if (ready_queue.empty() && !swapped_out.empty()) {
            _process *proc = swapped_out.front();
            swapped_out.pop();
            if (available_frames >= ESSENTIAL_PAGES) {
                swap_in(proc);
            } else {
                swapped_out.push(proc);
                break;
            }
            continue;
        }

        _process *current = ready_queue.front();
        ready_queue.pop();

        if (current->cur_search_idx >= m) {
            completed_count++;
            for (int i = 0; i < 2048; i++) {
                page_clear(page_tables, current->pid, i);
            }
            current->addi_data_seg = 0;
            if (!swapped_out.empty() && available_frames >= ESSENTIAL_PAGES) {
                _process *next = swapped_out.front();
                swapped_out.pop();
                swap_in(next);
            }
            continue;
        }

        int key = current->searches[current->cur_search_idx];
        int l = 0, r = current->max_A_size - 1;
        bool swapped_out_flag = false;

#ifdef VERBOSE
        printf("\tSearch %d by Process %d\n", current->cur_search_idx + 1, current->pid);
#endif
        while (l < r && !swapped_out_flag) {
            int m = (l + r) / 2;
            tot_page_accs++;
            int page = 10 + (m / 1024);
            if (!page_retrieve(page_tables, current->pid, page)) {
                tot_page_faults++;
                if (available_frames <= 0) {
                    swap_out(current);
                    swapped_out_flag = true;
                    break;
                }
                page_set(page_tables, current->pid, page);
            }
            if (key <= m) r = m;
            else l = m + 1;
        }

        if (!swapped_out_flag) {
            current->cur_search_idx++;
            if (current->cur_search_idx < m) {
                ready_queue.push(current);
            } else {
                completed_count++;
                for (int i = 0; i < 2048; i++) {
                    page_clear(page_tables, current->pid, i);
                }
                current->addi_data_seg = 0;
                if (!swapped_out.empty() && available_frames >= ESSENTIAL_PAGES) {
                    _process *next = swapped_out.front();
                    swapped_out.pop();
                    swap_in(next);
                }
            }
        }
    }

    printf("+++ Page access summary\n");
    printf("Total number of page accesses = %d\n", tot_page_accs);
    printf("Total number of page faults = %d\n", tot_page_faults);
    printf("Total number of swaps = %d\n", tot_swaps);
    printf("Degree of multiprogramming = %d\n", multi_prog);

    free(page_tables);
    free(process);
    return 0;
}