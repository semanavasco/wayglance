#ifndef DEF_SYSTEM_MODULE_HPP
#define DEF_SYSTEM_MODULE_HPP

#include "../vendor/json.hpp"
#include "BaseModule.hpp"
#include <string>

class SystemModule : public BaseModule {
public:
  SystemModule(const nlohmann::json &config);
  ~SystemModule();

private:
  // Widgets
  Gtk::Label m_cpu_label;
  Gtk::Label m_ram_label;
  Gtk::Label m_net_label;

  // Configuration
  unsigned int m_update_interval;
  bool m_cpu_active;
  bool m_ram_active;
  bool m_net_active;
  std::string m_cpu_format;
  std::string m_ram_format;
  std::string m_net_format;
  std::string m_net_interface;

  // State
  long m_prev_idle_time = 0;
  long m_prev_total_time = 0;

  long long m_prev_bytes_received = 0;
  long long m_prev_bytes_sent = 0;

  // Methods
  void load_config(const nlohmann::json &config);
  bool on_update_timer();
  void update_cpu_usage();
  void update_ram_usage();
  void update_net_usage();
  Glib::ustring format_label(const std::string &format, const std::string &key,
                             double value);
  Glib::ustring format_net_label(double download, double upload);
  Glib::ustring format_speed(double speed_bps);
};

#endif // !DEF_SYSTEM_MODULE_HPP
