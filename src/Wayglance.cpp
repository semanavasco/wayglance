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
    : m_modules_box(Gtk::Orientation::VERTICAL) {
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
  load_config();
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
  const auto css_path = m_config_dir_path / "style.css";
  bool load_from_file = true;

  if (!std::filesystem::exists(css_path)) {
    if (std::filesystem::exists(m_config_dir_path)) {
      try {
        std::ofstream css_file_stream(css_path);
        css_file_stream << default_css;
        std::cout << "Warning: A default style.css file has been created at "
                  << css_path << std::endl;
      } catch (const std::exception &e) {
        std::cerr << "Error: Couldn't create a style.css file, loading default "
                     "stylesheet : "
                  << e.what() << std::endl;
        load_from_file = false;
      }
    } else
      load_from_file = false;
  }

  // Set the appropriate css provider
  if (load_from_file) {
    css_provider->load_from_path(css_path.string());
    std::cout << "Loaded stylesheet from style.css file" << std::endl;
  } else {
    css_provider->load_from_data(default_css);
    std::cout << "Loaded default stylesheet" << std::endl;
  }

  // Applying the provider
  Gtk::StyleProvider::add_provider_for_display(
      get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void Wayglance::load_config() {
  // Defining the default config
  const std::string default_config = R"JSON({
  "modules": [ "date" ],
  "date": {
    "time_format": "%H:%M",
    "date_format": "%A, %d %B %Y"
  }
})JSON";

  bool load_from_mem = true;
  std::string json_content_to_parse = default_config;

  // Finding which config to load
  if (!m_config_dir_path.empty()) {
    const auto config_path = m_config_dir_path / "config.json";

    if (std::filesystem::exists(config_path)) {
      try {
        std::ifstream file_stream(config_path);
        json_content_to_parse.assign(
            (std::istreambuf_iterator<char>(file_stream)),
            (std::istreambuf_iterator<char>()));
        load_from_mem = false;
      } catch (const std::exception &e) {
        std::cerr << "Error: Couldn't read the existing config.json, using "
                     "default config : "
                  << e.what() << std::endl;
      }
    } else {
      try {
        std::ofstream config_file_stream(config_path);
        config_file_stream << default_config;
        std::cout << "Warning: A default config.json file has been created at "
                  << config_path << std::endl;
        load_from_mem = false;
      } catch (const std::exception &e) {
        std::cerr
            << "Error: Couldn't create a config.json file, loading default "
               "configuration : "
            << e.what() << std::endl;
      }
    }
  }

  // Parse the chosen json configuration
  try {
    m_config = nlohmann::json::parse(json_content_to_parse);
    if (!load_from_mem)
      std::cout << "Loaded configuration from config.json file" << std::endl;
    else
      std::cout << "Loaded default configuration" << std::endl;
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "Error: Couldn't parse config.json file, using default "
                 "configuration : "
              << e.what() << std::endl;

    m_config = nlohmann::json::parse(default_config);
  }
}

void Wayglance::load_modules() {
  m_modules_box.set_valign(Gtk::Align::CENTER);
  m_modules_box.set_halign(Gtk::Align::CENTER);

  if (m_config.contains("modules")) {
    for (const auto &module_name : m_config["modules"]) {
      if (module_name == "date") {
        nlohmann::json date_config =
            m_config.value("date", nlohmann::json::object());
        m_modules_box.append(*Gtk::make_managed<DateModule>(date_config));
      }
    }
  }
}

// Global Methods
void Wayglance::on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width,
                        int height) {
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  cr->paint();
}
