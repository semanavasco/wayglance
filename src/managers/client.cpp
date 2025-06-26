#include "managers/client.hpp"
#include "managers/config.hpp"
#include <iostream>
#include <unordered_set>

using namespace wayglance;

// Constructor
managers::Client::Client(Glib::RefPtr<Gtk::Application> app,
                         std::shared_ptr<Config> config_manager)
    : m_app(app), m_config_manager(config_manager) {}

// Destructor
managers::Client::~Client() {}

// Methods
void managers::Client::run() {
  // Getting the default display manager
  auto display = Gdk::Display::get_default();
  if (!display) {
    std::cerr << "Error: Could not get a GDK display" << std::endl;
    return;
  }

  // Connecting to monitors signal
  auto monitors = display->get_monitors();
  if (monitors) {
    monitors->signal_items_changed().connect(
        [this](guint position, guint removed, guint added) {
          std::cout << "Monitors configuration has changed, updating windows"
                    << added << std::endl;
          update_monitors();
        });
  }

  // Initial monitors configuration
  update_monitors();
}

void managers::Client::update_monitors() {
  // Getting the current monitors list
  auto display = Gdk::Display::get_default();
  if (!display)
    return;
  auto current_monitors_list = display->get_monitors();
  if (!current_monitors_list)
    return;

  // Creating a set of current monitors for efficient search
  std::unordered_set<GdkMonitor *> current_monitor_set;
  for (guint i = 0; i < current_monitors_list->get_n_items(); ++i) {
    auto monitor_obj = current_monitors_list->get_object(i);
    current_monitor_set.insert(
        reinterpret_cast<GdkMonitor *>(monitor_obj->gobj()));
  }

  // Removing monitors that are no longer available
  std::vector<Glib::RefPtr<Gdk::Monitor>> monitors_to_remove;
  for (const auto &pair : m_windows)
    if (current_monitor_set.find(pair.first) == current_monitor_set.end())
      monitors_to_remove.push_back(Glib::wrap(pair.first));

  for (const auto &monitor : monitors_to_remove)
    remove_monitor(monitor);

  // Adding missing windows
  for (guint i = 0; i < current_monitors_list->get_n_items(); ++i) {
    auto monitor_object = current_monitors_list->get_object(i);
    auto monitor = std::dynamic_pointer_cast<Gdk::Monitor>(monitor_object);
    if (monitor)
      add_monitor(monitor);
  }
}

void managers::Client::add_monitor(const Glib::RefPtr<Gdk::Monitor> &monitor) {
  if (m_windows.count(monitor->gobj())) {
    std::cout << "Warning: A window for this monitor already exists, ignoring"
              << std::endl;
    return;
  }

  std::cout << "Creating a Wayglance window for a monitor" << std::endl;

  // Creating a window instance
  auto window = new wayglance::Shell(m_config_manager, monitor->gobj());
  m_app->add_window(*window);
  window->show();

  // Store the window
  m_windows[monitor->gobj()] = window;
}

void managers::Client::remove_monitor(
    const Glib::RefPtr<Gdk::Monitor> &monitor) {
  std::cout << "Monitor removed signal received" << std::endl;

  // Find the window associated with the removed monitor
  auto it = m_windows.find(monitor->gobj());

  if (it != m_windows.end()) {
    wayglance::Shell *window_to_remove = it->second;
    std::cout << "Closing Wayglance window for the removed monitor"
              << std::endl;

    // Close the window (GTK manages its destruction)
    window_to_remove->close();

    // Remove the window from our managed windows
    m_windows.erase(it);
  }
}
