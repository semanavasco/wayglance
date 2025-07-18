#include "shell.hpp"
#include "modules/date.hpp"
#include "modules/player.hpp"
#include "modules/system.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <unordered_set>

extern "C" {
#include <gtk4-layer-shell.h>
}

// Constructor
wayglance::Shell::Shell(std::shared_ptr<managers::Config> config_manager, GdkMonitor *monitor)
    : m_config_manager(config_manager) {
  set_title("Wayglance");
  set_child(m_overlay);
  set_name("wayglance");

  // Overlay configuration
  setup_module_boxes();

  // Configuring layer shell
  gtk_layer_init_for_window((GtkWindow *)gobj());

  if (monitor)
    gtk_layer_set_monitor((GtkWindow *)gobj(), monitor);

  gtk_layer_set_layer((GtkWindow *)gobj(), GTK_LAYER_SHELL_LAYER_BACKGROUND);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);

  gtk_layer_set_keyboard_mode((GtkWindow *)gobj(), GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

  // Configuration
  Gtk::CssProvider::add_provider_for_display(get_display(), m_config_manager->get_provider(),
                                             GTK_STYLE_PROVIDER_PRIORITY_USER);
  try {
    load_modules();
  } catch (const std::exception &e) {
    spdlog::error("Couldn't load modules : {}", e.what());
  }
}

// Destructor
wayglance::Shell::~Shell() {}

// Methods
void wayglance::Shell::setup_module_boxes() {
  setup_module_box(m_top_left_box, "top-left", Gtk::Align::START, Gtk::Align::START);
  setup_module_box(m_top_center_box, "top-center", Gtk::Align::CENTER, Gtk::Align::START);
  setup_module_box(m_top_right_box, "top-right", Gtk::Align::END, Gtk::Align::START);

  setup_module_box(m_middle_left_box, "middle-left", Gtk::Align::START, Gtk::Align::CENTER);
  setup_module_box(m_middle_center_box, "middle-center", Gtk::Align::CENTER, Gtk::Align::CENTER);
  setup_module_box(m_middle_right_box, "middle-right", Gtk::Align::END, Gtk::Align::CENTER);

  setup_module_box(m_bottom_left_box, "bottom-left", Gtk::Align::START, Gtk::Align::END);
  setup_module_box(m_bottom_center_box, "bottom-center", Gtk::Align::CENTER, Gtk::Align::END);
  setup_module_box(m_bottom_right_box, "bottom-right", Gtk::Align::END, Gtk::Align::END);
}

void wayglance::Shell::setup_module_box(Gtk::Box &box, const std::string &name, Gtk::Align halign,
                                        Gtk::Align valign) {
  // Default configurations
  const auto &config = m_config_manager->get_config();
  Gtk::Orientation orientation = Gtk::Orientation::VERTICAL;
  int spacing = 0;

  // Looking for this box's configuration
  if (config.contains("layout") && config["layout"].contains(name)) {
    const auto &box_config = config["layout"][name];

    std::string orientation_str = box_config.value("orientation", "vertical");
    if (orientation_str == "horizontal")
      orientation = Gtk::Orientation::HORIZONTAL;

    spacing = box_config.value("spacing", 0);
  }

  // Configuring the box
  box.set_orientation(orientation);
  box.set_spacing(spacing);
  box.set_halign(halign);
  box.set_valign(valign);
  box.set_name(name + "-box");

  // Add the box to the overlay
  m_overlay.add_overlay(box);
}

void wayglance::Shell::load_modules() {
  auto config = m_config_manager->get_config();

  if (!config.contains("modules"))
    throw std::runtime_error("No modules list was found in the configuration file");

  std::unordered_set<std::string> loaded_modules;

  for (const auto &module_config : config["modules"]) {
    std::string name = module_config.value("name", "");
    std::string position = module_config.value("position", "middle-center");

    if (name.empty()) {
      spdlog::warn("Skipping malformed module entry");
      continue;
    }

    // Skip if module is already loaded
    const auto [_, inserted] = loaded_modules.emplace(name);
    if (!inserted) {
      spdlog::warn("Skipping duplicate module entry \"{}\"", name);
      continue;
    }

    Gtk::Box *target_box = &m_middle_center_box;

    if (position == "top-left")
      target_box = &m_top_left_box;
    else if (position == "top-center")
      target_box = &m_top_center_box;
    else if (position == "top-right")
      target_box = &m_top_right_box;
    else if (position == "middle-left")
      target_box = &m_middle_left_box;
    else if (position == "middle-center")
      target_box = &m_middle_center_box;
    else if (position == "middle-right")
      target_box = &m_middle_right_box;
    else if (position == "bottom-left")
      target_box = &m_bottom_left_box;
    else if (position == "bottom-center")
      target_box = &m_bottom_center_box;
    else if (position == "bottom-right")
      target_box = &m_bottom_right_box;
    else {
      spdlog::warn("Warning: Unrecognized module position \"{}\" for module "
                   "\"{}\", defaulting to middle-center",
                   position, name);
      target_box = &m_middle_center_box;
    }

    if (name == "date")
      target_box->append(*Gtk::make_managed<wayglance::modules::Date>(
          config.value("date", nlohmann::json::object())));
    else if (name == "player")
      target_box->append(*Gtk::make_managed<wayglance::modules::Player>(
          config.value("player", nlohmann::json::object())));
    else if (name == "system")
      target_box->append(*Gtk::make_managed<wayglance::modules::System>(
          config.value("system", nlohmann::json::object())));
    else
      spdlog::warn("Unrecognized module \"{}\" found, skipping it", name);
  }
}
