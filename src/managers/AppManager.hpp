#ifndef DEF_APP_MANAGER_HPP
#define DEF_APP_MANAGER_HPP

#include "../Wayglance.hpp"
#include "ConfigManager.hpp"
#include <gtkmm.h>
#include <map>
#include <memory>

class AppManager {
public:
  AppManager(Glib::RefPtr<Gtk::Application> app,
             std::shared_ptr<ConfigManager> config);
  ~AppManager();

  void run();

private:
  Glib::RefPtr<Gtk::Application> m_app;
  std::shared_ptr<ConfigManager> m_config_manager;
  std::map<GdkMonitor *, Wayglance *> m_windows;

  void add_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);
  void remove_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor);
  void update_monitors();
};

#endif // !DEF_APP_MANAGER_HPP
