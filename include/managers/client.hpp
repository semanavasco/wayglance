#pragma once

#include "config.hpp"
#include "gdkmm/display.h"
#include "gtkmm/application.h"
#include "shell.hpp"
#include <gtkmm.h>
#include <memory>
#include <unordered_map>

namespace wayglance::managers {

/**
 * @class Client
 * @brief Singleton client manager that handles the GTK application lifecycle
 * and monitor management.
 *
 * The Client class is responsible for:
 * - Managing the GTK application instance
 * - Handling command-line arguments and configuration
 * - Monitoring display changes and managing multiple monitors
 * - Creating and destroying Shell windows for each monitor
 */
class Client {
private:
  Client() = default;
  ~Client() = default;

  /**
   * @brief Sets up initial monitor detection and configures monitor change
   * callbacks.
   */
  void handle_monitors();

  /**
   * @brief Updates the list of monitors and creates/removes Shell windows as
   * needed.
   */
  void update_monitors();

  /**
   * @brief Creates a new Shell window for the specified monitor.
   * @param monitor The monitor to create a Shell window for.
   */
  void add_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);

  /**
   * @brief Removes the Shell window associated with the specified monitor.
   * @param monitor The monitor whose Shell window should be removed.
   */
  void remove_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);

  Glib::RefPtr<Gtk::Application> m_gtk_app;
  Glib::RefPtr<Gdk::Display> m_gdk_display;
  std::shared_ptr<Config> m_config_manager;
  std::unordered_map<GdkMonitor *, std::unique_ptr<Shell>> m_windows;

public:
  /**
   * @brief Gets the singleton instance of the Client.
   * @return Reference to the singleton Client instance.
   */
  static Client &inst();

  /**
   * @brief Runs the application with the given command-line arguments.
   * @param argc Number of command-line arguments.
   * @param argv Array of command-line arguments.
   * @return Exit code (0 for success, non-zero for failure).
   */
  int run(int argc, char *argv[]);
};

} // namespace wayglance::managers
