#include "Wayglance.hpp"
#include "modules/DateModule.hpp"
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>

extern "C" {
#include <gtk4-layer-shell.h>
}

// Constructor
Wayglance::Wayglance(GdkMonitor *p_monitor)
    : m_modules_box(Gtk::Orientation::VERTICAL), m_date_module() {
  // Window configuration
  set_title("Wayglance");
  set_child(m_overlay);
  set_name("wallpaper_window");

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
  setup_paths();
  load_style();
  load_modules();
}

// Destructor
Wayglance::~Wayglance() {}

// Configuration Methods
void Wayglance::setup_paths() {
  // Determining which configuration path to use
  std::filesystem::path config_base_path;
  const char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  const char *home_dir = getenv("HOME");

  // Use XDG_CONFIG_HOME if it exists
  if (xdg_config_home && std::strlen(xdg_config_home) > 0)
    config_base_path = xdg_config_home;
  // Use the HOME env var otherwise
  else {
    config_base_path = home_dir;
    config_base_path /= ".config";
  }

  if (config_base_path.empty()) {
    std::cerr << "Error: Couldn't find a config directory environment variable."
              << std::endl;
    return;
  }

  m_config_dir_path = config_base_path / "wayglance";

  if (std::filesystem::exists(m_config_dir_path)) {
    std::cout << "Found configuration directory at " << m_config_dir_path
              << std::endl;
    return;
  }

  try {
    std::cout << "Warning: Wayglance configuration directory does not exist, "
                 "attempting to create it."
              << std::endl;

    std::filesystem::create_directories(m_config_dir_path);
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Error: Couldn't create the configuration directory for "
                 "Wayglance : "
              << e.what() << std::endl;

    // Invalidating the config path
    m_config_dir_path.clear();
  }
}

void Wayglance::load_style() {
  // Defining the default style
  const std::string default_css = R"CSS(
    #wallpaper_window {
       background: none;
    }
    .time {
       font-family: "JetBrainsMono Nerd Font", "JetBrains Mono", monospace;
       font-size: 120pt;
       font-weight: bold;
       color: #cba6f7;
    }
    .date {
       font-family: "JetBrainsMono Nerd Font", "JetBrains Mono", monospace;
       font-size: 30pt;
       font-weight: normal;
       color: white;
    }
  )CSS";

  auto css_provider = Gtk::CssProvider::create();

  // Finding which style to load
  if (!m_config_dir_path.empty()) {
    std::filesystem::path css_path = m_config_dir_path / "style.css";

    if (!std::filesystem::exists(css_path)) {
      try {
        std::cout << "Warning: Stylesheet does not exist in the configuration "
                     "directory, attempting to create a default one"
                  << std::endl;

        std::ofstream css_file_stream(css_path);
        css_file_stream << default_css;
        css_provider->load_from_path(css_path.string());
      } catch (std::filesystem::filesystem_error &e) {
        std::cerr << "Error: Couldn't create default stylesheet file : "
                  << e.what() << std::endl;
      }
    } else
      css_provider->load_from_path(css_path.string());
  } else
    css_provider->load_from_data(default_css);

  // Applying the provider
  Gtk::StyleProvider::add_provider_for_display(
      get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void Wayglance::load_modules() {
  m_modules_box.set_valign(Gtk::Align::CENTER);
  m_modules_box.set_halign(Gtk::Align::CENTER);

  m_modules_box.append(m_date_module);
}

// Global Methods
void Wayglance::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                        int height) {
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  cr->paint();
}
