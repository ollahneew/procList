#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <pwd.h> // Include pwd.h for getpwuid

using namespace std;

struct ProcessInfo {
    int pid;
    int ppid;
    string username; // Added username field
    string name;
    string cmdline;
};

vector<ProcessInfo> processes;

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

// Function to extract UID from the status file
int getUid(const string& statusPath) {
    ifstream statusFile(statusPath);
    string line;
    while (getline(statusFile, line)) {
        if (line.find("Uid:") == 0) {
            istringstream iss(line);
            string uidLabel;
            int uid;
            iss >> uidLabel >> uid;
            return uid;
        }
    }
    return -1; // Return -1 if UID is not found
}

// Function to get username from UID
string getUsername(int uid) {
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return string(pw->pw_name);
    }
    return "[unknown]";
}

int main() {
    DIR* procDir = opendir("/proc");
    if (!procDir) {
        cerr << "Failed to open /proc directory" << endl;
        return 1;
    }

    struct dirent* entry;
    while ((entry = readdir(procDir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            string dirName = entry->d_name;
            if (all_of(dirName.begin(), dirName.end(), ::isdigit)) {
                ProcessInfo info;
                info.pid = stoi(dirName);

                string statusPath = "/proc/" + dirName + "/status";
                int uid = getUid(statusPath); // Get UID
                info.username = getUsername(uid); // Get username from UID

                string cmdlinePath = "/proc/" + dirName + "/cmdline";
                ifstream cmdlineFile(cmdlinePath);
                if (cmdlineFile) {
                    getline(cmdlineFile, info.cmdline, '\0');
                    if (info.cmdline.empty()) {
                        info.cmdline = "[empty]";
                    }
                } else {
                    info.cmdline = "[unreadable - permission denied]";
                }

                processes.push_back(info);
            }
        }
    }

    closedir(procDir);

    // Output the collected process information in a table format
    cout << left << setw(10) << "PID" << setw(10) << "PPID" << setw(15) << "Username" << setw(20) << "Name" << "Cmdline" << endl;
    cout << string(75, '-') << endl;
    for (const auto& process : processes) {
        cout << left << setw(10) << process.pid << setw(10) << process.ppid << setw(15) << process.username
             << setw(20) << process.name << process.cmdline << endl;
    }

    return 0;
}
