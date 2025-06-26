#pragma once

#include "module.hpp"

namespace wayglance::modules {

class Date : public wayglance::Module {
public:
  Date(const nlohmann::json &config);
  ~Date();

private:
  // Configuration
  std::string m_time_format = "{:%H:%M}";
  std::string m_date_format = "{:%A %d %B %Y}";

  // General
  Gtk::Label m_time_label;
  Gtk::Label m_date_label;

  bool update_labels();
  void load_config(const nlohmann::json &config);
};

} // namespace wayglance::modules
