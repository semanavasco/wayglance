#ifndef DEF_CONFIG_MANAGER_HPP
#define DEF_CONFIG_MANAGER_HPP

#include "../vendor/json.hpp"
#include <filesystem>
#include <gtkmm.h>

class ConfigManager {
public:
  ConfigManager();
  ~ConfigManager();

  // Getters
  const nlohmann::json &get_config();
  Glib::RefPtr<Gtk::CssProvider> get_css_provider();

private:
  std::filesystem::path m_config_dir_path;
  nlohmann::json m_config;
  Glib::RefPtr<Gtk::CssProvider> m_style;

  // Methods
  void setup_paths();
  void ensure_file_exists(const std::filesystem::path path,
                          const std::string &default_content);
  void load_config();
  void load_style();
};

#endif // !DEF_CONFIG_MANAGER_HPP
