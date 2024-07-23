#include <fstream>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <vector>
#include <algorithm>

struct ProcessInfo {
    int pid;
    int ppid;
    int uid;
    std::string username;
    std::string cmdline;
};

// Function to get UID and PPID from /proc/PID/status
void getUidAndPPid(const std::string& statusPath, int& uid, int& ppid) {
    std::ifstream statusFile(statusPath);
    std::string line;
    uid = -1; 
    ppid = -1; 
    while (std::getline(statusFile, line)) {
        if (line.rfind("Uid:", 0) == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> uid; // Extract UID
        } else if (line.rfind("PPid:", 0) == 0) {
            std::istringstream iss(line);
            std::string key;
            iss >> key >> ppid; // Extract PPID
        }
        if (uid != -1 && ppid != -1) break; // Break if both UID and PPID are found
    }
}

// Function to get username from UID
std::string getUsername(int uid) {
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : "unknown";
}


void listProcesses() {
    // Open /proc directory
    DIR* procDir = opendir("/proc");
    struct dirent* entry;
    std::vector<ProcessInfo> processes;
    // Iterate over each entry in /proc
    while ((entry = readdir(procDir)) != nullptr) {
        // Check if entry is a directory
        if (entry->d_type == DT_DIR) {
            std::string dirName = entry->d_name;
            // Check if directory name is a number
            if (std::all_of(dirName.begin(), dirName.end(), ::isdigit)) {
                ProcessInfo info;
                info.pid = std::stoi(dirName); // Get PID
                std::string statusPath = "/proc/" + dirName + "/status";
                getUidAndPPid(statusPath, info.uid, info.ppid); // Get UID and PPID
                info.username = getUsername(info.uid); // Get username from UID
                processes.push_back(info); // Store process info
            }
        }
    }
    closedir(procDir)

    // Output
    cout << left << setw(10) << "PID" << setw(10) << "PPID" << setw(15) << "Username" << setw(20) << "Name" << "Cmdline" << endl;
    cout << string(75, '-') << endl;
    for (const auto& process : processes) {
        cout << left << setw(10) << process.pid << setw(10) << process.ppid << setw(15) << process.username
             << setw(20) << process.name << process.cmdline << endl;
    }

    return 0;
}
