#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_PATH 1024
#define MAX_CMDLINE 4096

void print_process_info(const char *pid) {
    char path[MAX_PATH];
    char cmdline[MAX_CMDLINE];
    char process_name[256];
    FILE *fp;
    struct passwd *pw;
    uid_t uid;
    
    // Đọc thông tin process
    snprintf(path, sizeof(path), "/proc/%s/stat", pid);
    fp = fopen(path, "r");
    if (fp) {
        int ppid;
        unsigned long long starttime;
        fscanf(fp, "%*d (%[^)]) %*c %d %*d %*d %*d %*d %*u %*u %*u %*u %*u %*u %*u %*d %*d %*d %*d %*d %*d %llu",
               process_name, &ppid, &starttime);
        fclose(fp);
        
        // Đọc Command Line
        snprintf(path, sizeof(path), "/proc/%s/cmdline", pid);
        fp = fopen(path, "r");
        if (fp) {
            size_t size = fread(cmdline, sizeof(char), sizeof(cmdline) - 1, fp);
            if (size > 0) {
                if (cmdline[size - 1] == '\n') {
                    cmdline[size - 1] = '\0';
                } else {
                    cmdline[size] = '\0';
                }
                for (size_t i = 0; i < size; i++) {
                    if (cmdline[i] == '\0') cmdline[i] = ' ';
                }
            } else {
                strcpy(cmdline, process_name);  // Sử dụng tên process nếu cmdline trống
            }
            fclose(fp);
            
            // Đọc UID của process
            snprintf(path, sizeof(path), "/proc/%s/status", pid);
            fp = fopen(path, "r");
            if (fp) {
                char line[256];
                while (fgets(line, sizeof(line), fp)) {
                    if (sscanf(line, "Uid: %d", &uid) == 1) {
                        break;
                    }
                }
                fclose(fp);
                
                // Lấy tên người dùng từ UID
                pw = getpwuid(uid);
                
                // In thông tin process
                printf("%-40s %-10s %-15s %-10d %-30s\n", process_name, pid, pw ? pw->pw_name : "Unknown", ppid, cmdline);
            }
        }
    }
}

int main() {
    DIR *dir;
    struct dirent *ent;
    printf("%-40s %-10s %-15s %-10s %-30s\n", "Name", "PID", "User", "PPID", "Cmdline");
    dir = opendir("/proc");
    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (ent->d_type == DT_DIR) {
                char *endptr;
                long pid = strtol(ent->d_name, &endptr, 10);
                if (*endptr == '\0') {
                    print_process_info(ent->d_name);
                }
            }
        }
        closedir(dir);
    } else {
        perror("Không thể mở thư mục /proc");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}