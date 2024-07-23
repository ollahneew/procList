#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

struct ProcessInfo {
    int pid;
    int ppid;
    int uid;
    string username;
    string name;
    string cmdline;
};
// Open /proc/<PID>/status file and get UID and PPID
void getUidAndPPid(const string& statusPath, int& uid, int& ppid) {
    ifstream statusFile(statusPath);
    string line;
    while (getline(statusFile, line)) {
        if (line.substr(0, 4) == "Uid:") {
            istringstream iss(line);
            string temp;
            iss >> temp >> uid; // Get UID
        } else if (line.substr(0, 5) == "PPid:") {
            istringstream iss(line);
            string temp;
            iss >> temp >> ppid; // Get PPID
        }
    }
}
// Get username from UID
string getUsername(int uid) {
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return string(pw->pw_name);
    }
    return "N/A";
}

int main() {
    vector<ProcessInfo> processes;
    
    DIR *procDir = opendir("/proc");// Open /proc directory
    if (!procDir) {
        cerr << "Failed to open /proc directory" << endl;
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(procDir)) != nullptr) {  // Read /proc directory
        if (entry->d_type == DT_DIR) { // Check if entry is a directory
            string dirName = entry->d_name;
            if (all_of(dirName.begin(), dirName.end(), ::isdigit)) { // Check if directory name is a number
                ProcessInfo info;
                info.pid = stoi(dirName); // Get PID
                string statusPath = "/proc/" + dirName + "/status";
                getUidAndPPid(statusPath, info.uid, info.ppid); // Get UID and PPID
                info.username = getUsername(info.uid); // Get username from UID
                processes.push_back(info); 
            }
        }
    }
    closedir(procDir);

    cout << left << setw(10) << "PID" << setw(10) << "PPID" << setw(15) << "Username" << setw(20) << "Name" << "Cmdline" << endl;
    cout << string(75, '-') << endl;
    for (const auto& process : processes) {
        cout << left << setw(10) << process.pid << setw(10) << process.ppid << setw(15) << process.username
             << setw(20) << process.name << process.cmdline << endl;
    }

    return 0;
}