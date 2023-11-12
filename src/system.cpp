#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "process.h"
#include "processor.h"
#include "system.h"
#include "linux_parser.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;
using std::sort;

/*You need to complete the mentioned TODOs in order to satisfy the rubric criteria "The student will be able to extract and display basic data about the system."

You need to properly format the uptime. Refer to the comments mentioned in format. cpp for formatting the uptime.*/

// TODO: Return the system's CPU
// CPU usage is calculated in processor.cpp
Processor& System::Cpu() { return cpu_; }

// TODO: Return a container composed of the system's processes
// but only top 10 processes with most CPU usage will be displayed in the system monitor
vector<Process>& System::Processes() {
/* The next line resets the vector processes_.  If you don't reset it, you get the same PID in every row of the monitor.  */
  processes_ = {};
  vector<int> pids = LinuxParser::Pids();
  for (int pid : pids) {
/* The next line is same as processes_.emplace_back(Process(pid));
That's because when you emplace_back the pid by processes_.emplace_back(pid), constructor of the class Process gets called and since we already have a constructor with definition which takes a pid and returns the Process Object. */
    processes_.emplace_back(pid);
  }
/* The next line sorts the processes in descending order of CPU usage so the process with the most CPU usage is displayed at the top.  Instead of using
sort(processes_.begin(), processes_.end(), Compare());
which requires creating a Compare() function, I used the Lambda function, which has the format [ ]( ){ }.  Inside the square brackets [ ], we include the values to capture from the current scope.  Inside () are the arguments that get passed to the function (what you would pass to the Compare() function), inside {} is the body of the function (what would be the body of the Compare() function).  The function is return b < a, where the operator < has been overloaded in Process::operator<(). */
  sort(processes_.begin(), processes_.end(), []
      (Process a, Process b){ return b < a; });
  return processes_;
}

// TODO: Return the system's kernel identifier (string)
std::string System::Kernel() const {
  return LinuxParser::Kernel();
}

// TODO: Return the system's memory utilization
float System::MemoryUtilization() {
  return LinuxParser::MemoryUtilization();
}

// TODO: Return the operating system name
std::string System::OperatingSystem() const {
  return LinuxParser::OperatingSystem();
}

// TODO: Return the number of processes actively running on the system
int System::RunningProcesses() {
  return LinuxParser::RunningProcesses();
}

// TODO: Return the total number of processes on the system
int System::TotalProcesses() {
  return LinuxParser::TotalProcesses();
}

// TODO: Return the number of seconds since the system started running
long int System::UpTime() {
  return LinuxParser::UpTime();
}
