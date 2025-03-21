#include <bits/stdc++.h>
using namespace std;
#include <pthread.h>

int n, m; // n threads, m resources
pthread_barrier_t BOS;
pthread_barrier_t REQB;
pthread_barrier_t *ACKB;
queue<pair<int,vector<int>>> que;
pthread_mutex_t rmtx;
pthread_mutex_t pmtx; 
pthread_mutex_t *cond_mtx;
pthread_cond_t *cond_var;
int *done;

vector<vector<int>> need;
vector<int> available;  
vector<vector<int>> allocation;
vector<int> total;

typedef struct{
    char type;
    int id;
    vector<int> req;
}requests;

requests *sharedmem;

bool banker_algo(vector<vector<int>> &alloc, vector<vector<int>> &need, vector<int> &avail, vector<int> &req, int &id) {
    vector<int> temp_avail = avail;
    vector<vector<int>> temp_alloc = alloc;
    vector<vector<int>> temp_need = need;
    for (int i = 0; i < m; i++) {
        temp_avail[i] -= req[i];           // Reduce available resources
        temp_alloc[id][i] += req[i];       // Increase allocation for thread id
        temp_need[id][i] -= req[i];        // Decrease need for thread id
    }
    vector<int> work = temp_avail;         // Work vector starts as available
    vector<bool> finish(n, false);         // Track which threads can finish
    
    int finished = 0;
    while (finished < n) {
        bool found = false;
        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                bool can_finish = true;
                for (int j = 0; j < m; j++) {
                    if (temp_need[i][j] > work[j]) {
                        can_finish = false;
                        break;
                    }
                }
                if (can_finish) {
                    for (int j = 0; j < m; j++) {
                        work[j] += temp_alloc[i][j];
                    }
                    finish[i] = true;
                    finished++;
                    found = true;
                }
            }
        }
        if (!found) return false;
    }
    return true;
}

void *user_thread(void *arg) {
    int id = *((int *)arg); 
    pthread_mutex_lock(&pmtx);
    printf("\tThread %2d born\n", id); fflush(stdout);
    pthread_mutex_unlock(&pmtx);
    pthread_barrier_wait(&BOS);  

    string filename;
    if (id < 10) {
        filename = "./input/thread0" + std::to_string(id) + ".txt";
    } else {
        filename = "./input/thread" + std::to_string(id) + ".txt";
    }
    FILE *file = fopen(filename.c_str(), "r");
    if (file == NULL) {
        pthread_mutex_lock(&pmtx);
        printf("Error: Could not open file %s\n", filename.c_str()); fflush(stdout);
        pthread_mutex_unlock(&pmtx);
        return nullptr;
    }
    vector<int> req(m);
    vector<int> max_req(m);
    vector<int> cur_holding(m);
    for (int i = 0; i < m; i++) {
        fscanf(file, "%d", &max_req[i]);
        need[id][i] = max_req[i];
    }
    int delay;
    char charii;
    char type;
    while (1) {
        fscanf(file, "%d", &delay);
        fscanf(file, " %c", &charii); 
        usleep(delay * 1000);
        if (charii == 'Q') {
            type = 'Q';
            for(int i = 0; i < m; i++) {
                req[i] = -cur_holding[i];
                cur_holding[i] = 0;
            }
        } else {
            type = 'R';
            for (int i = 0; i < m; i++) {
                fscanf(file, "%d", &req[i]);
                if (req[i] > 0) type = 'A';
            }
            pthread_mutex_lock(&pmtx);
            printf("\tThread %2d sends resource request: type = %s\n", id, type == 'A' ? "ADDITIONAL" : "RELEASE"); fflush(stdout);
            pthread_mutex_unlock(&pmtx);
        }
        pthread_mutex_lock(&rmtx);
        sharedmem->type = type;
        sharedmem->id = id;
        sharedmem->req = req;
        pthread_barrier_wait(&REQB);
        pthread_barrier_wait(&ACKB[id]);
        pthread_mutex_unlock(&rmtx);
        if (charii == 'Q') {
            pthread_mutex_lock(&pmtx);
            printf("\tThread %2d going to quit\n", id); fflush(stdout);
            pthread_mutex_unlock(&pmtx);
            break;
        }
        if (type == 'A') {
            pthread_mutex_lock(&cond_mtx[id]);
            while (done[id] == 0) {
                pthread_cond_wait(&cond_var[id], &cond_mtx[id]);
            }
            pthread_mutex_lock(&pmtx);
            printf("\tThread %2d is granted its last resource request\n", id); fflush(stdout);
            pthread_mutex_unlock(&pmtx);
            done[id] = 0;
            pthread_mutex_unlock(&cond_mtx[id]);
        }
        for (int i = 0; i < m; i++) {
            cur_holding[i] += req[i];
        }
    }

    fclose(file);
    return nullptr;
}

int main() {
    int total_finished = 0;
    FILE *file = fopen("./input/system.txt", "r");
    if (file == NULL) {
        pthread_mutex_lock(&pmtx);
        printf("Error: Could not open file\n"); fflush(stdout);
        pthread_mutex_unlock(&pmtx);
        return 1;
    }
    fscanf(file, "%d", &m);  
    fscanf(file, "%d", &n); 
    total.resize(m);
    available.resize(m);  
    allocation.resize(n);
    need.resize(n);
    for (int i = 0; i < n; i++) {
        need[i].resize(m);
        allocation[i].resize(m);
    }
    for (int i = 0; i < m; i++) {
        fscanf(file, "%d", &total[i]);
        available[i] = total[i];
    }
    fclose(file);
    sharedmem = (requests *)malloc(sizeof(requests));
    pthread_barrier_init(&BOS, NULL, n + 1);
    pthread_barrier_init(&REQB, NULL, 2);
    ACKB = (pthread_barrier_t *)malloc(n * sizeof(pthread_barrier_t));
    cond_mtx = (pthread_mutex_t *)malloc(n * sizeof(pthread_mutex_t));
    cond_var = (pthread_cond_t *)malloc(n * sizeof(pthread_cond_t));
    done = (int *)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        pthread_barrier_init(&ACKB[i], NULL, 2);
        cond_mtx[i] = PTHREAD_MUTEX_INITIALIZER;
        cond_var[i] = PTHREAD_COND_INITIALIZER;
        done[i] = 0;
    }
    pthread_mutex_init(&rmtx, NULL);
    pthread_mutex_init(&pmtx, NULL); 

    pthread_t th[n];
    for (int i = 0; i < n; i++) {
        int *id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&th[i], nullptr, user_thread, (void *)id);
    }
    pthread_barrier_wait(&BOS);

    char next_req_type;
    int next_req_id;
    vector<int> next_req(m);
    while (1) {
        pthread_barrier_wait(&REQB);
        next_req_type = sharedmem->type;
        next_req_id = sharedmem->id;
        next_req = sharedmem->req;
        pthread_barrier_wait(&ACKB[next_req_id]);
        
        for (int i = 0; i < m; i++) {
            if (next_req[i] < 0) {
                available[i] -= next_req[i];
                allocation[next_req_id][i] += next_req[i];
                need[next_req_id][i] -= next_req[i];
                next_req[i] = 0;
            }
        }
        if (next_req_type == 'Q') {
            total_finished++;
            pthread_mutex_lock(&pmtx);
            printf("Master thread releases resources of thread %2d\n", next_req_id); fflush(stdout);
            if (!que.empty()) {
                printf("\t\tWaiting threads:");
                queue<pair<int, vector<int>>> temp = que;
                while (!temp.empty()) {
                    printf(" %d", temp.front().first);
                    temp.pop();
                }
                printf("\n"); fflush(stdout);
            } else {
                printf("\t\tWaiting threads:\n"); fflush(stdout);
            }
            printf("%d threads left\n", n - total_finished); fflush(stdout);
            printf("Available resources: ");
            for (int i = 0; i < m; i++) {
                printf("%d ", available[i]);
            }
            printf("\n"); fflush(stdout);
            pthread_mutex_unlock(&pmtx);
            if (total_finished == n) break;
        } else if (next_req_type == 'A') {
            pthread_mutex_lock(&pmtx);
            printf("Master thread stores resource request of thread %2d\n", next_req_id); fflush(stdout);
            pthread_mutex_unlock(&pmtx);
            que.push({next_req_id, next_req});
        }

        pthread_mutex_lock(&pmtx);
        if (!que.empty()) {
            printf("\t\tWaiting threads:");
            queue<pair<int, vector<int>>> temp = que;
            while (!temp.empty()) {
                printf(" %d", temp.front().first);
                temp.pop();
            }
            printf("\n"); fflush(stdout);
        } else {
            printf("\t\tWaiting threads:\n"); fflush(stdout);
        }
        pthread_mutex_unlock(&pmtx);
        
        pthread_mutex_lock(&pmtx);
        printf("Master thread tries to grant pending requests\n"); fflush(stdout);
        pthread_mutex_unlock(&pmtx);
        queue<pair<int, vector<int>>> temp_que;
        while (!que.empty()) {
            pair<int, vector<int>> p = que.front();
            que.pop();
            int id = p.first;
            vector<int> req = p.second;
            bool flag = true;
            for (int i = 0; i < m; i++) {
                if (req[i] > available[i]) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                #ifdef _DLAVOID
                flag = banker_algo(allocation, need, available, req, id);
                #endif
            }
            if (flag) {
                pthread_mutex_lock(&pmtx);
                printf("Master thread grants resource request for thread %2d\n", id); fflush(stdout);
                pthread_mutex_unlock(&pmtx);
                for (int i = 0; i < m; i++) {
                    available[i] -= req[i];
                    allocation[id][i] += req[i];
                    need[id][i] -= req[i];
                }
                pthread_mutex_lock(&cond_mtx[id]);
                done[id] = 1;
                pthread_cond_signal(&cond_var[id]);
                pthread_mutex_unlock(&cond_mtx[id]);
            } else {
                pthread_mutex_lock(&pmtx);
                printf("    +++ Insufficient resources to grant request of thread %2d\n", id); fflush(stdout);
                pthread_mutex_unlock(&pmtx);
                temp_que.push(p);
            }
        }
        que = temp_que;

        pthread_mutex_lock(&pmtx);
        if (!que.empty()) {
            printf("\t\tWaiting threads:");
            queue<pair<int, vector<int>>> temp = que;
            while (!temp.empty()) {
                printf(" %d", temp.front().first);
                temp.pop();
            }
            printf("\n"); fflush(stdout);
        } else {
            printf("\t\tWaiting threads:\n"); fflush(stdout);
        }
        pthread_mutex_unlock(&pmtx);
    }

    for (int i = 0; i < n; i++) {
        pthread_join(th[i], nullptr);
        pthread_barrier_destroy(&ACKB[i]);
        pthread_mutex_destroy(&cond_mtx[i]);
        pthread_cond_destroy(&cond_var[i]);
    }

    pthread_barrier_destroy(&BOS);
    pthread_barrier_destroy(&REQB);
    pthread_mutex_destroy(&rmtx);
    pthread_mutex_destroy(&pmtx); 
    free(sharedmem);
    free(ACKB);
    free(cond_mtx);
    free(cond_var);
    free(done);

    return 0;
}