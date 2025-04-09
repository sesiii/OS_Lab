#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <errno.h>

// Structure to store UID to username mapping
#define MAX_USERS 1000
struct user_map {
    uid_t uid;
    char username[32];
} users[MAX_USERS];
int user_count = 0;

void load_passwd() {
    FILE *passwd = fopen("/etc/passwd", "r");
    if (!passwd) {
        fprintf(stderr, "Error opening /etc/passwd: %s\n", strerror(errno));
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), passwd) && user_count < MAX_USERS) {
        char *username = strtok(line, ":");
        strtok(NULL, ":");
        char *uid_str = strtok(NULL, ":");
        
        if (username && uid_str) {
            users[user_count].uid = atoi(uid_str);
            strncpy(users[user_count].username, username, 31);
            users[user_count].username[31] = '\0';
            user_count++;
        }
    }
    fclose(passwd);
}

// Function to convert UID to username
const char *uid_to_username(uid_t uid) {
    for (int i = 0; i < user_count; i++) {
        if (users[i].uid == uid) {
            return users[i].username;
        }
    }
    static char uid_str[16];
    snprintf(uid_str, sizeof(uid_str), "%u", uid);
    return uid_str; 
}

// Recursive function to search directory
void search_dir(const char *dname, const char *ext, int *serial) {
    // printf("NO : OWNER SIZE NAME");
    DIR *dir = opendir(dname);
    if (!dir) {
        fprintf(stderr, "Cannot open directory %s: %s\n", dname, strerror(errno));
        return;
    }

    struct dirent *entry;
    char path[PATH_MAX];
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(path, PATH_MAX, "%s/%s", dname, entry->d_name);

        struct stat st;
        if (stat(path, &st) == -1) {
            fprintf(stderr, "Cannot stat %s: %s\n", path, strerror(errno));
            continue;
        }

        if (S_ISREG(st.st_mode)) { 
            
            const char *file_ext = strrchr(entry->d_name, '.');
            if (file_ext && strcmp(file_ext, ext) == 0) {
                printf("%-6d: %-10s %-12ld %s\n",
                    (*serial)++,
                    uid_to_username(st.st_uid),
                    (long)st.st_size,
                    path);
                // printf("%d   : %s     %ld     %s\n", (*serial)++, uid_to_username(st.st_uid), (long)st.st_size, path);
            }
            
        } else if (S_ISDIR(st.st_mode)) {
            search_dir(path, ext, serial);
        }
    }
    // printf("+++%d files match the extension\n",serial);
    
    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <directory> <extension>\n", argv[0]);
        return 1;
    }
    printf("NO    : OWNER      SIZE         NAME\n");
    printf("--      -----      ----         ----\n");
    char ext[32];
    if (argv[2][0] != '.') {
        snprintf(ext, sizeof(ext), ".%s", argv[2]);
    } else {
        strncpy(ext, argv[2], sizeof(ext));
        ext[sizeof(ext)-1] = '\0';
    }

    load_passwd();

    int serial = 1;
    search_dir(argv[1], ext, &serial);
    printf("+++%d files match the extension\n",serial-1);
    return 0;
}