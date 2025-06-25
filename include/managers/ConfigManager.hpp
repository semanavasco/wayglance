#ifndef DEF_CONFIG_MANAGER_CPP
#define DEF_CONFIG_MANAGER_CPP

#include "glibmm/refptr.h"
#include "gtkmm/cssprovider.h"
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace fs = std::filesystem;

/**
 * @class ConfigManager
 * @brief The ConfigManager handles configuration loading, creation and default
 * fallbacks.
 */
class ConfigManager {
public:
  ConfigManager();
  ~ConfigManager();

  // --- Methods ---
  /**
   * @brief Loads the configuration.
   * * Parses the available json configuration file or fallbacks to defaults.
   * The config is stored in the m_config.
   * * Loads the available css stylesheet file or fallbacks to defaults. The
   * stylesheet is stored in the m_provider.
   */
  void load();

  /**
   * @brief Creates default configuration files, overriding the current
   * configuration if existing.
   * @return true if the creation succeeded, false otherwise.
   */
  bool create_defaults();

  // --- Setters ---
  /**
   * @brief Sets the configuration file path to use.
   * @param path The path to use as config.json
   * @return true if it was applied, false otherwise.
   */
  bool set_custom_config_path(const std::string &path);

  /**
   * @brief Sets the stylesheet file path to use.
   * @param path The path to use as style.css
   * @return true if it was applied, false otherwise.
   */
  bool set_custom_style_path(const std::string &path);

  // --- Getters ---
  /**
   * @brief Get the parsed configuration.
   */
  const nlohmann::json &get_config();

  /**
   * @brief Get the css provider.
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

#endif // !DEF_CONFIG_MANAGER_CPP
