#pragma once

#include "module.hpp"
#include "sigc++/connection.h"
#include <string>

namespace wayglance::modules {

/**
 * @class System
 * @brief A system monitoring module that displays CPU usage, RAM usage, and
 * network statistics.
 *
 * The System module provides real-time monitoring with configurable update
 * intervals of:
 * - CPU usage percentage
 * - RAM usage (used/total) with customizable formatting
 * - Network statistics (download/upload speeds) for specified interface
 * - Individual enable/disable options for each monitoring component
 * - Configurable display formats for all metrics
 */
class System : public wayglance::Module {
public:
  /**
   * @brief Constructs a System module with the given configuration.
   * @param config JSON configuration object containing monitoring settings and
   * format options.
   */
  System(const nlohmann::json &config);

  /**
   * @brief Destructor for the System module.
   */
  ~System();

private:
  // Widgets
  Gtk::Label m_cpu_label;
  Gtk::Label m_ram_label;
  Gtk::Label m_net_label;

  // Configuration
  unsigned int m_update_interval;
  bool m_cpu_active;
  bool m_ram_active;
  bool m_net_active;
  std::string m_cpu_format;
  std::string m_ram_format;
  std::string m_net_format;
  std::string m_net_interface;

  // State
  sigc::connection m_update_timer;

  long m_prev_idle_time = 0;
  long m_prev_total_time = 0;

  long long m_prev_bytes_received = 0;
  long long m_prev_bytes_sent = 0;

  // Methods
  /**
   * @brief Loads configuration settings for system monitoring.
   * @param config JSON configuration object containing update intervals,
   * formats, and active components.
   */
  void load_config(const nlohmann::json &config);

  /**
   * @brief Timer callback that updates all active system metrics.
   * @return true to continue the timer, false to stop it.
   */
  bool on_update_timer();

  /**
   * @brief Updates the CPU usage percentage by reading /proc/stat.
   */
  void update_cpu_usage();

  /**
   * @brief Updates the RAM usage information by reading /proc/meminfo.
   */
  void update_ram_usage();

  /**
   * @brief Updates the network usage statistics by reading /proc/net/dev.
   */
  void update_net_usage();

  /**
   * @brief Formats a metric label using a template string.
   * @param format Format template string with placeholder markers.
   * @param key The placeholder key to replace in the format string.
   * @param value The numeric value to insert.
   * @return Formatted label string.
   */
  Glib::ustring format_label(const std::string &format, const std::string &key, double value);

  /**
   * @brief Formats the network usage label with download and upload speeds.
   * @param download Download speed in bytes per second.
   * @param upload Upload speed in bytes per second.
   * @return Formatted network usage string.
   */
  Glib::ustring format_net_label(double download, double upload);

  /**
   * @brief Converts bytes per second to a human-readable speed format.
   * @param speed_bps Speed in bytes per second.
   * @return Human-readable speed string (e.g., "1.5 MB/s").
   */
  Glib::ustring format_speed(double speed_bps);
};

} // namespace wayglance::modules
