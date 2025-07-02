#include "managers/config.hpp"
#include "glibmm/error.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <spdlog/spdlog.h>
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
      "next": { "icon": "media-skip-forward-symbolic" },
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
managers::Config::Config() {
  // Determining which configuration path to use
  fs::path config_base_path;
  const char *xdg_config_home = getenv("XDG_CONFIG_HOME");

  // Use XDG_CONFIG_HOME if it exists
  if (xdg_config_home && std::strlen(xdg_config_home) > 0)
    config_base_path = xdg_config_home;
  // Use the HOME env var otherwise
  else {
    const char *home_dir = getenv("HOME");
    if (home_dir) {
      config_base_path = home_dir;
      config_base_path /= ".config";
    }
  }

  if (!config_base_path.empty())
    m_wayglance_path = config_base_path / "wayglance";
}

// Destructor
managers::Config::~Config() {}

// Methods
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
    spdlog::info("Loaded \"{}\" configuration", config_path.string());
  } catch (const std::exception &e) {
    spdlog::error("Couldn't read \"{}\" file : {}", config_path.string(),
                  e.what());
    m_config = nlohmann::json::parse(DEFAULT_CONFIG);
    spdlog::info("Loaded default configuration");
  }

  // Loading style.css
  if (style_path.empty() && !m_wayglance_path.empty())
    style_path = m_wayglance_path / "style.css";

  try {
    if (!style_path.empty() && fs::exists(style_path)) {
      m_provider->load_from_path(style_path);
      spdlog::info("Loaded \"{}\" stylesheet", style_path.string());
    } else {
      m_provider->load_from_data(std::string(DEFAULT_STYLE));
      spdlog::info("Loaded default stylesheet");
    }
  } catch (const Glib::Error &e) {
    spdlog::error("Couldn't load stylesheet \"{}\": {}", style_path.string(),
                  e.what());
    m_provider->load_from_data(std::string(DEFAULT_STYLE));
    spdlog::info("Loaded default stylesheet as fallback");
  }
}

fs::path managers::Config::create_defaults() {
  if (m_wayglance_path.empty())
    throw std::runtime_error(
        "Couldn't find default configuration path. Check your $XDG_CONFIG_HOME "
        "or $HOME environment variables");

  try {
    if (!fs::exists(m_wayglance_path)) {
      fs::create_directories(m_wayglance_path);
      spdlog::debug("Created directory: {}", m_wayglance_path.string());
    }
  } catch (const fs::filesystem_error &e) {
    throw std::runtime_error("Failed to create directory \"" +
                             m_wayglance_path.string() + "\": " + e.what());
  }

  fs::path config_path = m_wayglance_path / "config.json";
  fs::path style_path = m_wayglance_path / "style.css";

  spdlog::info("Creating \"{}\" file...", config_path.string());
  bool config = create_default_file(config_path, DEFAULT_CONFIG);
  if (config)
    spdlog::info("Ok");
  else
    throw std::runtime_error("Couldn't create config.json file");

  spdlog::info("Creating \"{}\" file...", style_path.string());
  bool style = create_default_file(style_path, DEFAULT_STYLE);
  if (style)
    spdlog::info("Ok");
  else
    throw std::runtime_error("Couldn't create style.css file");

  return m_wayglance_path;
}

// Setters
void managers::Config::set_custom_config_path(const std::string &path) {
  if (!fs::exists(path))
    throw std::runtime_error("Path \"" + path + "\" does not exist");

  if (fs::is_directory(path))
    throw std::runtime_error("Path \"" + path +
                             "\" is a directory, expected a file");

  if (!path.ends_with(".json"))
    throw std::runtime_error(
        "Path \"" + path +
        "\" doesn't have .json extension, expected a json file");

  m_custom_config_path = path;
}

void managers::Config::set_custom_style_path(const std::string &path) {
  if (!fs::exists(path))
    throw std::runtime_error("Path \"" + path + "\" does not exist");

  if (fs::is_directory(path))
    throw std::runtime_error("Path \"" + path +
                             "\" is a directory, expected a file");

  if (!path.ends_with(".css"))
    throw std::runtime_error(
        "Path \"" + path +
        "\" doesn't have .css extension, expected a css file");

  m_custom_style_path = path;
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
