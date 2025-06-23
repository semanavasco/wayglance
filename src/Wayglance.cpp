#include "Wayglance.hpp"
#include "modules/DateModule.hpp"
#include "modules/PlayerModule.hpp"
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
    : m_config_manager(config_manager),
      m_modules_box(Gtk::Orientation::VERTICAL) {
  // Window configuration
  set_title("Wayglance");
  set_child(m_overlay);
  set_name("wayglance");

  // DrawingArea configuration
  m_drawing_area.set_draw_func(sigc::mem_fun(*this, &Wayglance::on_draw));

  // Overlay configuration
  m_overlay.set_child(m_drawing_area);
  m_overlay.add_overlay(m_modules_box);

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

// Configuration Methods
void Wayglance::load_modules() {
  m_modules_box.set_valign(Gtk::Align::CENTER);
  m_modules_box.set_halign(Gtk::Align::CENTER);

  auto config = m_config_manager->get_config();

  if (!config.contains("modules")) {
    std::cerr << "Error: No modules list was found in the config" << std::endl;
    return;
  }

  std::unordered_set<std::string> loaded_modules;

  for (const auto &module_name_json : config["modules"]) {
    std::string module_name = module_name_json.get<std::string>();

    // Skip if module is already loaded
    const auto [_, inserted] = loaded_modules.emplace(module_name);
    if (!inserted) {
      std::cerr << "Warning: Skipping duplicate module entry '" << module_name
                << "'" << std::endl;
      continue;
    }

    if (module_name == "date")
      m_modules_box.append(*Gtk::make_managed<DateModule>(
          config.value("date", nlohmann::json::object())));
    else if (module_name == "player")
      m_modules_box.append(*Gtk::make_managed<PlayerModule>(
          config.value("player", nlohmann::json::object())));
    else
      std::cerr << "Warning: Unrecognized module '" << module_name
                << "' found, skipping it" << std::endl;
  }
}

// Global Methods
void Wayglance::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                        int height) {
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  cr->paint();
}
