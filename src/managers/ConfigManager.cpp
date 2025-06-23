#include "ConfigManager.hpp"
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

// Constants
const std::string DEFAULT_CONFIG = R"JSON({
  "modules": [
    { "name": "date", "position": "middle-center" },
    { "name": "player", "position": "middle-center" }
  ],
  "date": {
    "time_format": "%H:%M",
    "date_format": "%A, %d %B %Y"
  },
  "player": {
    "player": "spotify",
    "nerd-font": false,
    "buttons": {
      "previous": { "icon": "media-skip-backward-symbolic" },
      "next": { "icon": "media-skip-backward-symbolic" },
      "play": { "icon": "media-playback-start-symbolic" },
      "pause": { "icon": "media-playback-pause-symbolic" }
    }
  }
})JSON";

const std::string DEFAULT_STYLE = R"CSS(#wayglance {
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
})CSS";

// Constructor
ConfigManager::ConfigManager() {
  setup_paths();
  load_config();
  load_style();
}

// Destructor
ConfigManager::~ConfigManager() {}

// Getters
const nlohmann::json &ConfigManager::get_config() { return m_config; }

Glib::RefPtr<Gtk::CssProvider> ConfigManager::get_css_provider() {
  return m_style;
}

// Methods
void ConfigManager::setup_paths() {
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

void ConfigManager::ensure_file_exists(const std::filesystem::path path,
                                       const std::string &default_content) {
  if (!std::filesystem::exists(path)) {
    std::cout << "Warning: Couldn't find " << path
              << ", attempting to create it with default values" << std::endl;

    try {
      std::filesystem::create_directories(path.parent_path());
      std::ofstream file_stream(path);
      file_stream << default_content;
    } catch (const std::exception &e) {
      std::cerr << "Error: Couldn't create default file " << path << " : "
                << e.what() << std::endl;
    }
  }
}

void ConfigManager::load_config() {
  if (m_config_dir_path.empty()) {
    std::cerr << "Error: Configuration directory was not found, loading "
                 "default configuration"
              << std::endl;
    m_config = nlohmann::json::parse(DEFAULT_CONFIG);
    return;
  }

  const auto config_path = m_config_dir_path / "config.json";
  ensure_file_exists(config_path, DEFAULT_CONFIG);

  try {
    std::ifstream file_stream(config_path);
    m_config = nlohmann::json::parse(file_stream);
    std::cout << "Loaded " << config_path << " configuration" << std::endl;
    return;
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "Error: Couldn't parse " << config_path
              << " file : " << e.what() << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Error: Couldn't read " << config_path
              << " file : " << e.what() << std::endl;
  }

  m_config = nlohmann::json::parse(DEFAULT_CONFIG);
  std::cout << "Loaded default configuration" << std::endl;
}

void ConfigManager::load_style() {
  m_style = Gtk::CssProvider::create();

  if (m_config_dir_path.empty()) {
    std::cerr << "Error: Configuration directory was not found, loading "
                 "default stylesheet"
              << std::endl;
    m_style->load_from_data(DEFAULT_STYLE);
    return;
  }

  const auto css_path = m_config_dir_path / "style.css";
  ensure_file_exists(css_path, DEFAULT_STYLE);

  m_style->load_from_path(css_path.string());
  std::cout << "Loaded " << css_path << " stylesheet" << std::endl;
}
