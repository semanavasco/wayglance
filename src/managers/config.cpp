#include "managers/config.hpp"
#include "glibmm/error.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

constexpr std::string_view DEFAULT_CONFIG = R"JSON({
  "modules": [
    { "name": "date", "position": "middle-center" },
    { "name": "player", "position": "middle-center" },
    { "name": "system", "position": "bottom-center" }
  ],
  "date": {
    "h-align": "center",
    "v-align": "center",
    "orientation": "vertical",
    "spacing": 0,
    "time_format": "%H:%M",
    "date_format": "%A, %d %B %Y"
  },
  "player": {
    "h-align": "center",
    "v-align": "center",
    "orientation": "vertical",
    "spacing": 0,
    "player": "spotify",
    "nerd-font": false,
    "buttons": {
      "previous": { "icon": "media-skip-backward-symbolic" },
      "next": { "icon": "media-skip-backward-symbolic" },
      "play": { "icon": "media-playback-start-symbolic" },
      "pause": { "icon": "media-playback-pause-symbolic" }
    }
  },
  "system": {
    "h-align": "center",
    "v-align": "center",
    "orientation": "horizontal",
    "spacing": 10,
    "update-interval": 1000,
    "cpu": { "active": true, "format": "CPU: {usage}%" },
    "ram": { "active": true, "format": "RAM: {usage}%" },
    "net": { "active": true, "format": "NET: {download} / {upload}", "interface": "wlan0" }
  }
})JSON";

constexpr std::string_view DEFAULT_STYLE = R"CSS(#wayglance {
  background: none;
}

#date-time-label {
  font-size: 120pt;
  font-weight: bold;
  color: #cba6f7;
}
#date-date-label {
  font-size: 30pt;
  font-weight: normal;
  color: white;
}

#module-player {
  margin-top: 50pt;
}
.player-labels {
  font-size: 15pt;
}
#player-track-label {
  margin-bottom: 15pt;
}
.player-buttons {
  background-color: transparent;
}
.player-buttons:focus {
  outline: none;
}
.player-buttons:hover {
  background-color: rgba(255, 255, 255, 0.1);
}
#player-progress-bar {
  min-height: 6px;
  min-width: 400px;
}
#player-progress-bar progress {
  background-color: #cba6f7;
  border-radius: 4px;
}
#player-progress-bar trough {
  background-color: rgba(255, 255, 255, 0.2);
  border-radius: 4px;
}

#module-system {
  margin-bottom: 15pt;
})CSS";

} // namespace

using namespace wayglance;

// Constructor
managers::Config::Config() { setup(); }

// Destructor
managers::Config::~Config() {}

// Methods
void managers::Config::setup() {
  // Determining which configuration path to use
  fs::path config_base_path;
  const char *xdg_config_home = getenv("XDG_CONFIG_HOME");

  // Use XDG_CONFIG_HOME if it exists
  if (xdg_config_home && std::strlen(xdg_config_home) > 0)
    config_base_path = xdg_config_home;
  // Use the HOME env var otherwise
  else {
    const char *home_dir = getenv("HOME");
    if (home_dir && std::strlen(home_dir) > 0) {
      config_base_path = home_dir;
      config_base_path /= ".config";
    }
  }

  if (config_base_path.empty() || !fs::exists(config_base_path))
    return;

  // Setting m_wayglance_path
  fs::path wayglance_path = config_base_path / "wayglance";
  if (fs::exists(wayglance_path))
    m_wayglance_path = wayglance_path;
}

void managers::Config::load() {
  fs::path config_path = m_custom_config_path;
  fs::path style_path = m_custom_style_path;
  m_provider = Gtk::CssProvider::create();

  // Laoding config.json
  if (config_path.empty() && !m_wayglance_path.empty())
    config_path = m_wayglance_path / "config.json";

  try {
    std::ifstream file_stream(config_path);
    if (!file_stream)
      throw std::runtime_error("Couldn't open configuration file at \"" +
                               config_path.string() + "\"");

    m_config = nlohmann::json::parse(file_stream);
    std::cout << "Loaded " << config_path << " configuration" << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: Couldn't read " << config_path
              << " file : " << e.what() << std::endl;
    m_config = nlohmann::json::parse(DEFAULT_CONFIG);
    std::cout << "Loaded default configuration" << std::endl;
  }

  // Loading style.css
  if (style_path.empty() && !m_wayglance_path.empty())
    style_path = m_wayglance_path / "style.css";

  try {
    if (fs::exists(style_path)) {
      m_provider->load_from_path(style_path);
      std::cout << "Loaded " << style_path << " stylesheet" << std::endl;
    } else {
      m_provider->load_from_data(std::string(DEFAULT_STYLE));
      std::cout << "Loaded default stylesheet" << std::endl;
    }
  } catch (const Glib::Error &e) {
    std::cerr << "Error: Coudln't load stylesheet: " << e.what() << std::endl;
    m_provider->load_from_data(std::string(DEFAULT_STYLE));
    std::cout << "Loaded default stylesheet" << std::endl;
  }
}

bool managers::Config::create_defaults() {
  fs::path config_path = m_wayglance_path / "config.json";
  fs::path style_path = m_wayglance_path / "style.css";

  std::cout << "Creating " << config_path << " file... ";
  bool config = create_default_file(config_path, DEFAULT_CONFIG);
  if (config)
    std::cout << "Ok" << std::endl;
  else
    std::cerr << std::endl
              << "Error: Couldn't create config.json file. Check your "
                 "$XDG_CONFIG_HOME or $HOME environment variables."
              << std::endl;

  std::cout << "Creating " << style_path << " file... ";
  bool style = create_default_file(style_path, DEFAULT_STYLE);
  if (style)
    std::cout << "Ok" << std::endl;
  else
    std::cerr << std::endl
              << "Error: Couldn't create style.css file. Check your "
                 "$XDG_CONFIG_HOME or $HOME environment variables."
              << std::endl;

  return config && style;
}

// Setters
bool managers::Config::set_custom_config_path(const std::string &path) {
  if (!fs::exists(path))
    return false;

  m_custom_config_path = path;
  return true;
}

bool managers::Config::set_custom_style_path(const std::string &path) {
  if (!fs::exists(path))
    return false;

  m_custom_style_path = path;
  return true;
}

// Getters
const nlohmann::json &managers::Config::get_config() { return m_config; }

Glib::RefPtr<Gtk::CssProvider> managers::Config::get_provider() {
  return m_provider;
}

// Helpers
bool managers::Config::create_default_file(const fs::path &path,
                                           std::string_view content) {
  if (!fs::exists(path.parent_path()))
    fs::create_directories(path.parent_path());

  std::ofstream output_file(path);
  if (!output_file)
    return false;

  output_file << content;
  return true;
}
