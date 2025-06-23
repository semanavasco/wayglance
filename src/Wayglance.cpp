#include "Wayglance.hpp"
#include "modules/DateModule.hpp"
#include "modules/PlayerModule.hpp"
#include "modules/SystemModule.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>
#include <unordered_set>

extern "C" {
#include <gtk4-layer-shell.h>
}

// Constructor
Wayglance::Wayglance(std::shared_ptr<ConfigManager> config_manager,
                     GdkMonitor *p_monitor)
    : m_config_manager(config_manager) {
  set_title("Wayglance");
  set_child(m_overlay);
  set_name("wayglance");

  // DrawingArea configuration
  m_drawing_area.set_draw_func(sigc::mem_fun(*this, &Wayglance::on_draw));

  // Overlay configuration
  m_overlay.set_child(m_drawing_area);
  setup_module_boxes();

  // Configuring layer shell
  gtk_layer_init_for_window((GtkWindow *)gobj());

  if (p_monitor)
    gtk_layer_set_monitor((GtkWindow *)gobj(), p_monitor);

  gtk_layer_set_layer((GtkWindow *)gobj(), GTK_LAYER_SHELL_LAYER_BACKGROUND);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);

  gtk_layer_set_keyboard_mode((GtkWindow *)gobj(),
                              GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

  // Configuration
  Gtk::CssProvider::add_provider_for_display(
      get_display(), m_config_manager->get_css_provider(),
      GTK_STYLE_PROVIDER_PRIORITY_USER);
  load_modules();
}

// Destructor
Wayglance::~Wayglance() {}

// Methods
void Wayglance::setup_module_boxes() {
  setup_module_box(m_top_left_box, "top-left", Gtk::Align::START,
                   Gtk::Align::START);
  setup_module_box(m_top_center_box, "top-center", Gtk::Align::CENTER,
                   Gtk::Align::START);
  setup_module_box(m_top_right_box, "top-right", Gtk::Align::END,
                   Gtk::Align::START);

  setup_module_box(m_middle_left_box, "middle-left", Gtk::Align::START,
                   Gtk::Align::CENTER);
  setup_module_box(m_middle_center_box, "middle-center", Gtk::Align::CENTER,
                   Gtk::Align::CENTER);
  setup_module_box(m_middle_right_box, "middle-right", Gtk::Align::END,
                   Gtk::Align::CENTER);

  setup_module_box(m_bottom_left_box, "bottom-left", Gtk::Align::START,
                   Gtk::Align::END);
  setup_module_box(m_bottom_center_box, "bottom-center", Gtk::Align::CENTER,
                   Gtk::Align::END);
  setup_module_box(m_bottom_right_box, "bottom-right", Gtk::Align::END,
                   Gtk::Align::END);
}

void Wayglance::setup_module_box(Gtk::Box &box, const std::string &name,
                                 Gtk::Align halign, Gtk::Align valign) {
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

void Wayglance::load_modules() {
  auto config = m_config_manager->get_config();

  if (!config.contains("modules")) {
    std::cerr << "Error: No modules list was found in the config" << std::endl;
    return;
  }

  std::unordered_set<std::string> loaded_modules;

  for (const auto &module_config : config["modules"]) {
    std::string name = module_config.value("name", "");
    std::string position = module_config.value("position", "middle-center");

    // Skip if module is already loaded
    const auto [_, inserted] = loaded_modules.emplace(name);
    if (!inserted) {
      std::cerr << "Warning: Skipping duplicate module entry '" << name << "'"
                << std::endl;
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

    if (name == "date")
      target_box->append(*Gtk::make_managed<DateModule>(
          config.value("date", nlohmann::json::object())));
    else if (name == "player")
      target_box->append(*Gtk::make_managed<PlayerModule>(
          config.value("player", nlohmann::json::object())));
    else if (name == "system")
      target_box->append(*Gtk::make_managed<SystemModule>(
          config.value("system", nlohmann::json::object())));
    else
      std::cerr << "Warning: Unrecognized module '" << name
                << "' found, skipping it" << std::endl;
  }
}

void Wayglance::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                        int height) {
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  cr->paint();
}
