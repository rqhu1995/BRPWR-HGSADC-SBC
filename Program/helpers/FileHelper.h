#include "Genetic.h"
#include "Individual.h"
#include "Params.h"
#include "helpers/Args.h"
#include <ctime>
#include <iostream>
#include <string>
#include <sys/stat.h> // Include the stat.h library

#if defined(_WIN32)
#include <direct.h> // For _mkdir on Windows
#else
#include <filesystem>
#include <unistd.h> // For access() on Unix
#endif

namespace FileHelper {
// Function to check if a directory exists
inline bool directoryExists(const std::string &dir) {
#if defined(_WIN32)
  struct _stat info;
  if (_stat(dir.c_str(), &info) != 0) {
    return false; // Directory does not exist
  } else if (info.st_mode & _S_IFDIR) {
    return true; // Directory exists
  }
#else
  struct stat info;
  if (stat(dir.c_str(), &info) != 0) {
    return false; // Directory does not exist
  } else if (info.st_mode & S_IFDIR) {
    return true; // Directory exists
  }
#endif
  return false; // Path exists but it's not a directory
}

// Function to create a directory if it does not exist, including any necessary
// parent directories
inline bool createDirectoryRecursive(const std::string &dir) {
  std::string currentPath = "";
  std::size_t pos = 0;

  // Process each directory level
  do {
    pos = dir.find_first_of("/\\", pos + 1);
    currentPath = dir.substr(0, pos);
    if (currentPath.empty())
      continue; // If leading /, first token will be empty

    if (!directoryExists(currentPath)) {
#if defined(_WIN32)
      if (_mkdir(currentPath.c_str()) != 0 && errno != EEXIST) {
        std::cerr << "Error: Unable to create directory " << currentPath
                  << std::endl;
        return false;
      }
#else
      if (mkdir(currentPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) !=
              0 &&
          errno != EEXIST) {
        std::cerr << "Error: Unable to create directory " << currentPath
                  << std::endl;
        return false;
      }
#endif
    }
  } while (pos != std::string::npos);

  return true;
}

inline void saveResult(Individual &bestSol, Params &params, int numStations,
                       int instanceNum, double prop, Genetic &genetic) {
  time_t now = time(0);
  tm *ltm = localtime(&now);
  // Ensure that the date and time values are always in two digits
  char dirName[64];
  sprintf(dirName, "../Solutions/%04d-%02d-%02d/", 1900 + ltm->tm_year,
          1 + ltm->tm_mon, ltm->tm_mday);

  // Create the directory if it doesn't exist
  if (FileHelper::createDirectoryRecursive(dirName)) {
    int timeBudget = (int)Args::timeBudget / 3600;
    // round the prop to 2 decimal places
    float roundedProp = roundf(prop * 100) / 100;
    std::string fileName = std::string(dirName) + std::to_string(numStations) +
                           "_" + std::to_string(instanceNum) + "_t" +
                           std::to_string(Args::nbTrk) + "_r" +
                           std::to_string(Args::nbRpm) + "_" +
                           std::to_string(timeBudget) + "h_";
    // find how many files start with fileName in the directory
    int count = 0;
    for (const auto &entry : std::filesystem::directory_iterator(dirName)) {
      if (entry.path().string().find(fileName) != std::string::npos) {
        count++;
      }
    }
    fileName += std::to_string(count + 1) + ".txt";
    genetic.saveResults(bestSol, fileName);
  } else {
    std::cerr << "Failed to create directory: " << dirName << std::endl;
  }
}
} // namespace FileHelper