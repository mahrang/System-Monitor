#include <dirent.h>
#include <unistd.h>  // required to use sysconf(_SC_CLK_TCK)
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::stol;

string LinuxParser::GetValue(string file, string key_) {
  std::ifstream stream(kProcDirectory + file);
  string line, key, value;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == key_) { return value; }
      }
    }
  }
  return value;
}

vector<long> LinuxParser::ProcessJiffies(int pid) {
  string line, key, key_string;
  long key_long, seconds;
  vector<long> process_nums;
  std::ifstream stream(kProcDirectory + to_string(pid) +
                       kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    for (int i=0; i < 22; i++) {
      linestream >> key;
/* key_long = stol(key_string) will compile, but program will abort at runtime.  Problem is b/c I do emplace_back afterwards.  That's why I used the next 2 lines to convert key from string to long. */
      std::istringstream key_string(key);
      key_string >> key_long;
/* #s stored in key_long are in clock ticks.  Divide by sysconf(_SC_CLK_TCK) to get seconds. Need #include <unistd.h> to use sysconf(_SC_CLK_TCK). */
      seconds = key_long / sysconf(_SC_CLK_TCK);
      process_nums.emplace_back(seconds);
    }
  }
  return process_nums;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
/* According to https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc-meminfo-like-htop/41251290#41251290
Total memory used = MemTotal - MemFree */
  float MemTotal = stof(GetValue(kMeminfoFilename,
                                  "MemTotal:"));
  float MemFree = stof(GetValue(kMeminfoFilename,
                                  "MemFree:"));
  return (MemTotal - MemFree) / MemTotal;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  string line, uptime;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return stol(uptime);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return ActiveJiffies() + IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid) {
  vector<long> process_nums = ProcessJiffies(pid);
/* Read https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
to see why we use these 4 values in process_nums.
Also read "Part 3 - Project - System Monitor" file under the heading "Processor Utilization" to understand why we use these #s.
Also take a look at
https://man7.org/linux/man-pages/man5/proc.5.html
under the heading "/proc/[pid]/stat" to understand the #s. */
  return process_nums[13] + process_nums[14] +
         process_nums[15] + process_nums[16];
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> jiffies = CpuUtilization();
/* See https://stackoverflow.com/questions/23367857/accurate-calculation-of-cpu-usage-given-in-percentage-in-linux
for CPU calculation.  Active = NonIdle */
  return stol(jiffies[CPUStates::kUser_]) +
         stol(jiffies[CPUStates::kNice_]) +
         stol(jiffies[CPUStates::kSystem_]) +
         stol(jiffies[CPUStates::kIRQ_]) +
         stol(jiffies[CPUStates::kSoftIRQ_]) +
         stol(jiffies[CPUStates::kSteal_]);
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> jiffies = CpuUtilization();
  return stol(jiffies[CPUStates::kIdle_]) +
         stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string line, cpu, jiffy;
  vector<string> jiffies;
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> cpu;
    while (linestream >> jiffy) {
      // each jiffy is placed at the end of jiffies
      jiffies.emplace_back(jiffy);
    }
  }
  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  return stoi(GetValue(kStatFilename, "processes"));
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  return stoi(GetValue(kStatFilename, "procs_running"));
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) +
                       kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
  }
  return line;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  return to_string(stol(GetValue(to_string(pid) +
                       kStatusFilename, "VmSize:")) / 1000);
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  return GetValue(to_string(pid) + kStatusFilename, "Uid:");
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string line, user, x, uid;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> x >> uid) {
        if (uid == Uid(pid)) {
          std::replace(line.begin(), line.end(), ' ', ':');
          return user;
        }
      }
    }
  }
  return user;
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::UpTime(int pid) {
  vector<long> process_nums = ProcessJiffies(pid);
/* process_nums[21] is the start time of the process in seconds. Read https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
to see why we use process_nums[21].
Also read "Part 3 - Project - System Monitor" file under the heading "Up Time" under the heading "Process Data" to understand why we use this #.
Also take a look at
https://man7.org/linux/man-pages/man5/proc.5.html
under the heading "/proc/[pid]/stat" to understand this #. */
  return process_nums[21];
}
