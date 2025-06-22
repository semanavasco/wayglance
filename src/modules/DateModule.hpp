#ifndef DEF_DATE_MODULE_HPP
#define DEF_DATE_MODULE_HPP

#include "../vendor/json.hpp"
#include <gtkmm.h>

class DateModule : public Gtk::Box {
public:
  DateModule(const nlohmann::json &config);
  ~DateModule();

protected:
  // Configuration
  std::string m_time_format = "{:%H:%M}";
  std::string m_date_format = "{:%A %d %B %Y}";

  // General
  Gtk::Label m_time_label;
  Gtk::Label m_date_label;

  bool update_labels();
  void load_config(const nlohmann::json &config);
};

#endif // !DEF_DATE_MODULE_HPP
