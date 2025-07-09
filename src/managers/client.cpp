#include "managers/client.hpp"
#include "gdkmm/display.h"
#include "managers/config.hpp"
#include <cstdlib>
#include <gdk/wayland/gdkwayland.h>
#include <iostream>
#include <lyra/lyra.hpp>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>

using namespace wayglance;

namespace {
constexpr std::string_view WAYGLANCE_VERSION = "0.0.36";
}

// Methods
managers::Client &managers::Client::inst() {
  static Client instance;
  return instance;
}

int managers::Client::run(int argc, char *argv[]) {
  // Options values
  bool show_help = false;
  bool show_version = false;
  bool create_defaults = false;
  std::string log_level;
  std::string config_path;
  std::string style_path;

  // Options
  auto cli =
      lyra::cli() | lyra::help(show_help) |
      lyra::opt(show_version)["-v"]["--version"](
          "Show Wayglance version and exit") |
      lyra::opt(
          log_level,
          "trace|debug|info|warning|error|critical|off")["-l"]["--log-level"](
          "Defines the log level to display") |
      lyra::opt(config_path, "path")["-c"]["--config"](
          "Overrides default config path (default: "
          "$XDG_CONFIG_HOME/wayglance/config.json or "
          "$HOME/.config/wayglance/config.json)") |
      lyra::opt(style_path, "path")["-s"]["--style"](
          "Overrides default style path (default: "
          "$XDG_CONFIG_HOME/wayglance/style.css or "
          "$HOME/.config/wayglance/style.css)") |
      lyra::opt(create_defaults)["-d"]["--create-defaults"](
          "Creates a default configuration at "
          "$XDG_CONFIG_HOME/wayglance or $HOME/.config/wayglance");

  auto result = cli.parse({argc, argv});
  if (!result) {
    spdlog::error("Command line arguments error: {}", result.message());
    return EXIT_FAILURE;
  }

  if (show_help) {
    std::cout << cli << std::endl;
    return EXIT_SUCCESS;
  }

  if (show_version) {
    std::cout << "Wayglance " << WAYGLANCE_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (!log_level.empty())
    spdlog::set_level(spdlog::level::from_str(log_level));

  // Creating app
  m_gtk_app =
      Gtk::Application::create("io.github.semanavasco.wayglance",
                               Gtk::Application::Flags::HANDLES_COMMAND_LINE);

  m_gdk_display = Gdk::Display::get_default();
  if (!m_gdk_display)
    throw std::runtime_error("Couldn't find display.");

  if (!GDK_WAYLAND_DISPLAY(m_gdk_display->gobj()))
    throw std::runtime_error("Wayglance must run under Wayland");

  // Handling config
  m_config_manager = std::make_shared<managers::Config>();

  if (create_defaults) {
    try {
      fs::path config_path = m_config_manager->create_defaults();
      spdlog::info("Created default configuration files at \"{}\"",
                   config_path.string());
      return EXIT_SUCCESS;
    } catch (const std::exception &e) {
      spdlog::error("Couldn't create default configuration files : {}",
                    e.what());
      return EXIT_FAILURE;
    }
  }

  if (!config_path.empty()) {
    try {
      m_config_manager->set_custom_config_path(config_path);
      spdlog::info("Configuration path set to \"{}\"", config_path);
    } catch (const std::exception &e) {
      spdlog::error("Couldn't set custom configuration path : {}", e.what());
    }
  }

  if (!style_path.empty()) {
    try {
      m_config_manager->set_custom_style_path(style_path);
      spdlog::info("Stylesheet path set to \"{}\"", style_path);
    } catch (const std::exception &e) {
      spdlog::error("Couldn't set custom stylesheet path : {}", e.what());
    }
  }

  try {
    m_config_manager->load();
  } catch (const std::exception &e) {
    spdlog::error("Couldn't load configuration: {}", e.what());
    return EXIT_FAILURE;
  }

  // Start app
  m_gtk_app->register_application();
  m_gtk_app->activate();
  handle_monitors();
  return m_gtk_app->run(argc, argv);
}

void managers::Client::handle_monitors() {
  auto monitors = m_gdk_display->get_monitors();

  if (!monitors)
    throw std::runtime_error("Couldn't get monitors list");

  // Connect to monitors change signal
  monitors->signal_items_changed().connect(
      [this](guint position, guint removed, guint added) {
        update_monitors();
      });

  update_monitors();
}

void managers::Client::update_monitors() {
  // Getting the current monitors list
  auto current_monitors_list = m_gdk_display->get_monitors();
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
    spdlog::warn("A window for this monitor already exists, ignoring");
    return;
  }

  spdlog::info("Creating a Wayglance window for a monitor");

  // Creating a window instance
  auto window = std::make_unique<Shell>(m_config_manager, monitor->gobj());
  m_gtk_app->add_window(*window);
  window->show();

  // Store the window
  m_windows[monitor->gobj()] = std::move(window);
}

void managers::Client::remove_monitor(
    const Glib::RefPtr<Gdk::Monitor> &monitor) {
  spdlog::info("Monitor removed signal received");

  // Find the window associated with the removed monitor
  auto it = m_windows.find(monitor->gobj());

  if (it != m_windows.end()) {
    spdlog::info("Closing Wayglance window for the removed monitor");

    // Close the window (GTK manages its destruction)
    it->second->close();

    // Remove the window from our managed windows
    m_windows.erase(it);
  }
}
