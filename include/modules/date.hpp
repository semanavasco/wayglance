#pragma once

#include "module.hpp"
#include "sigc++/connection.h"

namespace wayglance::modules {

/**
 * @class Date
 * @brief A module that displays the current time and date with customizable
 * formatting.
 *
 * The Date module provides real-time display of time and date information with:
 * - Configurable time format (default: HH:MM)
 * - Configurable date format (default: Weekday DD Month YYYY)
 * - Automatic updates every second
 * - CSS styling support for time and date labels
 */
class Date : public wayglance::Module {
public:
  /**
   * @brief Constructs a Date module with the given configuration.
   * @param config JSON configuration object containing time_format and
   * date_format options.
   */
  Date(const nlohmann::json &config);

  /**
   * @brief Destructor for the Date module.
   */
  ~Date();

private:
  // Configuration
  std::string m_time_format = "{:%H:%M}";
  std::string m_date_format = "{:%A %d %B %Y}";

  // General
  Gtk::Label m_time_label;
  Gtk::Label m_date_label;
  sigc::connection m_update_timer;

  /**
   * @brief Updates the time and date labels with the current time.
   * @return true to continue the timer callback, false to stop it.
   */
  bool update_labels();

  /**
   * @brief Loads configuration settings for time and date formatting.
   * @param config JSON configuration object containing format strings.
   */
  void load_config(const nlohmann::json &config);
};

} // namespace wayglance::modules
