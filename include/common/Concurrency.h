#ifndef CONCURRENCY_H
#define CONCURRENCY_H

#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <sched.h>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>
#include <unistd.h>

class Concurrency {
public:

  /**
   * Returns the number of physical CPU sockets (packages) on the machine.
   * This reads from sysfs to determine unique physical package IDs.
   * @return The number of physical CPU sockets, or -1 on error.
   */
  static int get_num_physical_cpus() {
    std::set<int> socket_ids;
    int max_cpu = sysconf(_SC_NPROCESSORS_CONF) - 1;
    
    if (max_cpu < 0) {
      return -1;
    }
    
    // Iterate through all possible CPUs
    // Limit to 1024 CPUs to avoid excessive iteration
    for (int cpu = 0; cpu <= max_cpu && cpu < 1024; ++cpu) {
      char path[256];
      snprintf(path, sizeof(path),
               "/sys/devices/system/cpu/cpu%d/topology/physical_package_id",
               cpu);
      
      std::ifstream file(path);
      if (file.is_open()) {
        int socket_id;
        if (file >> socket_id) {
          socket_ids.insert(socket_id);
        }
        file.close();
      }
    }
    
    if (socket_ids.empty()) {
      // Fallback: if sysfs is not available, return -1 to indicate error
      return -1;
    }
    
    return static_cast<int>(socket_ids.size());
  }

  /**
   * Returns the number of physical cores on the machine.
   * This reads from sysfs to determine unique (socket_id, core_id) pairs.
   * Physical cores are distinct from logical CPUs (which include hyperthreads).
   * @return The number of physical cores, or -1 on error.
   */
  static int get_num_physical_cores() {
    std::set<std::pair<int, int>> core_pairs;
    int max_cpu = sysconf(_SC_NPROCESSORS_CONF) - 1;
    
    if (max_cpu < 0) {
      return -1;
    }
    
    // Iterate through all possible CPUs
    // Limit to 1024 CPUs to avoid excessive iteration
    for (int cpu = 0; cpu <= max_cpu && cpu < 1024; ++cpu) {
      char package_path[256];
      char core_path[256];
      snprintf(package_path, sizeof(package_path),
               "/sys/devices/system/cpu/cpu%d/topology/physical_package_id",
               cpu);
      snprintf(core_path, sizeof(core_path),
               "/sys/devices/system/cpu/cpu%d/topology/core_id",
               cpu);
      
      std::ifstream package_file(package_path);
      std::ifstream core_file(core_path);
      
      if (package_file.is_open() && core_file.is_open()) {
        int socket_id, core_id;
        if ((package_file >> socket_id) && (core_file >> core_id)) {
          core_pairs.insert(std::make_pair(socket_id, core_id));
        }
        package_file.close();
        core_file.close();
      }
    }
    
    if (core_pairs.empty()) {
      // Fallback: if sysfs is not available, return -1 to indicate error
      return -1;
    }
    
    return static_cast<int>(core_pairs.size());
  }

  /**
  * Returns the number of cores available on the machine.
  * @return The number of cores available on the machine.
  */
  static int get_num_cores() { return sysconf(_SC_NPROCESSORS_CONF); }

  /**
   * Returns the number of available cores that can be used by the current process.
   * @return The number of available cores that can be used by the current process.
   */
  static int get_num_available_cores() { return sysconf(_SC_NPROCESSORS_ONLN); }

  /**
   * Returns whether hyperthreading is enabled on the machine.
   * @return True if hyperthreading is enabled, false otherwise.
   */
  static bool is_hyperthreading_enabled() { return get_num_cores() > get_num_physical_cores(); }

  /**
   * Returns a mapping of physical cores to their logical CPUs.
   * Each physical core is identified by (socket_id, core_id) and maps to
   * a vector of logical CPU IDs that share that physical core.
   * @return Map from (socket_id, core_id) pairs to vectors of logical CPU IDs.
   *         Returns empty map on error.
   */
  static std::map<std::pair<int, int>, std::vector<int>> get_physical_core_mapping() {
    std::map<std::pair<int, int>, std::vector<int>> core_map;
    int max_cpu = sysconf(_SC_NPROCESSORS_CONF) - 1;
    
    if (max_cpu < 0) {
      return core_map;
    }
    
    // Iterate through all possible CPUs
    for (int cpu = 0; cpu <= max_cpu && cpu < 1024; ++cpu) {
      char package_path[256];
      char core_path[256];
      snprintf(package_path, sizeof(package_path),
               "/sys/devices/system/cpu/cpu%d/topology/physical_package_id",
               cpu);
      snprintf(core_path, sizeof(core_path),
               "/sys/devices/system/cpu/cpu%d/topology/core_id",
               cpu);
      
      std::ifstream package_file(package_path);
      std::ifstream core_file(core_path);
      
      if (package_file.is_open() && core_file.is_open()) {
        int socket_id, core_id;
        if ((package_file >> socket_id) && (core_file >> core_id)) {
          std::pair<int, int> core_key = std::make_pair(socket_id, core_id);
          core_map[core_key].push_back(cpu);
        }
        package_file.close();
        core_file.close();
      }
    }
    
    return core_map;
  }

  /**
   * Prints the mapping of logical CPUs to physical cores.
   * Shows which logical CPUs share the same physical core.
   * Useful for understanding CPU topology and hyperthreading layout.
   */
  static void print_physical_core_mapping() {
    auto core_map = get_physical_core_mapping();
    
    if (core_map.empty()) {
      std::cerr << "Unable to determine physical core mapping." << std::endl;
      return;
    }
    
    std::cout << "Physical Core to Logical CPU Mapping:" << std::endl;
    std::cout << "====================================" << std::endl;
    
    for (const auto& entry : core_map) {
      const auto& core_key = entry.first;
      const auto& logical_cpus = entry.second;
      
      std::cout << "Socket " << core_key.first 
                << ", Core " << core_key.second 
                << ": Logical CPUs [";
      
      for (size_t i = 0; i < logical_cpus.size(); ++i) {
        std::cout << logical_cpus[i];
        if (i < logical_cpus.size() - 1) {
          std::cout << ", ";
        }
      }
      std::cout << "]" << std::endl;
    }
  }

  /**
   * Returns the logical CPU ID that the current thread is currently executing on.
   * This is the logical CPU (hardware thread) ID, not the physical core ID.
   * Note: Without pinning, this can change at any moment as the scheduler may
   * move the thread to a different CPU.
   * @return The logical CPU ID (0-based), or -1 on error or if not supported.
   */
  static int get_current_core() {
    int cpu = sched_getcpu();
    // sched_getcpu() returns -1 on error, which we pass through
    // Callers should check for -1 if error handling is needed
    return cpu;
  }

   /**
   * Pins the current thread to a specific CPU core.
   * @param core_id The ID of the CPU core to pin the thread to.
   * @return True if the thread is pinned to the core, false otherwise.
   */
  static bool pin_to_core(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_t current_thread = pthread_self();
    int result =
        pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
    if (result != 0) {
      std::cerr << "Error setting affinity for core " << core_id << std::endl;
      return false;
    }
    return true;
  }

  /**
   * Checks if the current thread is pinned to a specific subset of cores.
   * Returns true if the affinity mask is NOT set to all available cores.
   * @return True if the thread is pinned to the core, false otherwise.
   */
  static bool is_thread_pinned() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    // Get the affinity mask for the current thread
    if (pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=
        0) {
      return false;
    }

    // Check how many CPUs are in the set
    int count = CPU_COUNT(&cpuset);

    // Also get the total number of configured processors
    int total_procs = sysconf(_SC_NPROCESSORS_CONF);
    if (total_procs < 0) {
      // Error getting processor count, assume not pinned
      return false;
    }

    // If the thread is allowed to run on fewer than all available processors,
    // we consider it "pinned" (restricted).
    return count < total_procs;
  }
};

#endif
