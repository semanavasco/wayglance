#ifndef DEF_CONFIG_MANAGER_HPP
#define DEF_CONFIG_MANAGER_HPP

#include "../vendor/json.hpp"
#include <filesystem>
#include <gtkmm.h>

class ConfigManager {
public:
  ConfigManager();
  ~ConfigManager();

  void setup_paths();
  void load_config();
  void load_style();

  // Getters
  const nlohmann::json &get_config();
  Glib::RefPtr<Gtk::CssProvider> get_css_provider();

  // Setters
  void set_custom_config_path(const std::string &path);
  void set_custom_style_path(const std::string &path);

private:
  std::filesystem::path m_custom_config_path;
  std::filesystem::path m_custom_style_path;
  std::filesystem::path m_config_dir_path;
  nlohmann::json m_config;
  Glib::RefPtr<Gtk::CssProvider> m_style;

  // Methods
  void ensure_file_exists(const std::filesystem::path path,
                          const std::string &default_content);
};

#endif // !DEF_CONFIG_MANAGER_HPP
