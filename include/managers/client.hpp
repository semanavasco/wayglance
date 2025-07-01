#pragma once

#include "config.hpp"
#include "shell.hpp"
#include <gtkmm.h>
#include <memory>
#include <unordered_map>

namespace wayglance::managers {

/**
 * @class Client
 * @brief The Client handles the application's state and windows.
 */
class Client {
public:
  Client(Glib::RefPtr<Gtk::Application> app, std::shared_ptr<Config> config);
  ~Client();

  /**
   * @brief Runs the application, connecting to the display, handles monitors,
   * etc.
   */
  void run();

private:
  Glib::RefPtr<Gtk::Application> m_app;
  std::shared_ptr<Config> m_config_manager;
  std::unordered_map<GdkMonitor *, std::unique_ptr<Shell>> m_windows;

  /**
   * @brief Creates a window for a monitor and adds it to the handled monitors
   * list.
   * @param A reference to the monitor to handle.
   */
  void add_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);

  /**
   * @brief Handles monitors removal, killing their window instance and removing
   * it from the handled monitors list.
   * @param monitor A reference to the monitor to handle.
   */
  void remove_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);

  /**
   * @brief Refreshes monitors on monitor update signal receive. Calls
   * add_monitor for new monitors and remove_monitor for missing ones.
   */
  void update_monitors();
};

} // namespace wayglance::managers
