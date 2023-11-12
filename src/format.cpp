#include <string>

#include "format.h"

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long times) {
  int hours = times / 3600;
  long minutes_in_seconds = times - hours * 3600;
  int minutes = minutes_in_seconds / 60;
  int seconds = minutes_in_seconds - minutes * 60;
  return XX(hours) + ":" + XX(minutes) + ":" + XX(seconds);
}

string Format::XX(int num) {
  if (num < 10)
    return "0" + to_string(num);
  else
    return to_string(num);
}
