#pragma once

#include "glibmm/refptr.h"
#include "gtkmm/cssprovider.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

namespace wayglance::managers {

/**
 * @class Config
 * @brief Configuration manager that handles loading, parsing, and providing
 * access to the application's configuration.
 *
 * The Config class manages all configuration aspects of Wayglance including:
 * - JSON configuration file loading and parsing
 * - CSS stylesheet loading and management
 * - Default configuration files creation
 * - Custom configuration and style path handling
 * - Fallback to default values when configuration is missing
 */
class Config {
public:
  /**
   * @brief Constructs a Config manager and initializes default paths.
   */
  Config();

  /**
   * @brief Destructor for the Config manager.
   */
  ~Config();

  // --- Methods ---
  /**
   * @brief Loads the configuration and stylesheet files.
   *
   * This method performs the following operations:
   * - Parses the JSON configuration file
   * - Loads the CSS stylesheet file
   * - Stores the parsed configuration in m_config and stylesheet in m_provider
   * - Handles fallback to defaults when files are not found or invalid
   */
  void load();

  /**
   * @brief Creates default configuration files, overriding existing files.
   * @return The path where the configuration files were created.
   * @throws std::runtime_error if configuration path cannot be determined or
   * files cannot be created.
   */
  fs::path create_defaults();

  // --- Setters ---
  /**
   * @brief Sets the configuration file path to use.
   * @param path The path to use as config.json
   * @throws std::runtime_error if the path doesn't exist, is a directory, or
   * doesn't have .json extension
   */
  void set_custom_config_path(const std::string &path);

  /**
   * @brief Sets the stylesheet file path to use.
   * @param path The path to use as style.css
   * @throws std::runtime_error if the path doesn't exist, is a directory, or
   * doesn't have .css extension
   */
  void set_custom_style_path(const std::string &path);

  // --- Getters ---
  /**
   * @brief Gets the parsed JSON configuration object.
   * @return Const reference to the loaded configuration.
   */
  const nlohmann::json &get_config();

  /**
   * @brief Gets the CSS provider containing the loaded stylesheet.
   * @return RefPtr to the GTK CSS provider with the loaded styles.
   */
  Glib::RefPtr<Gtk::CssProvider> get_provider();

private:
  fs::path m_custom_config_path;
  fs::path m_custom_style_path;
  fs::path m_wayglance_path;

  nlohmann::json m_config;
  Glib::RefPtr<Gtk::CssProvider> m_provider;

  // Helpers
  /**
   * @brief Helper method to create or replace a file with a default content.
   * @param path The path of the file
   * @param content The content to write to the file
   * @return true if the file was created or replaced, false otherwise.
   */
  bool create_default_file(const fs::path &path, std::string_view content);
};

} // namespace wayglance::managers
