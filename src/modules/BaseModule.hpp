#ifndef DEF_BASE_MODULE_HPP
#define DEF_BASE_MODULE_HPP

#include "../vendor/json.hpp"
#include <gtkmm.h>

class BaseModule : public Gtk::Box {
public:
  BaseModule(const nlohmann::json &config);
  virtual ~BaseModule();

protected:
  Gtk::Align string_to_align(const std::string &align);
};

#endif // !DEF_BASE_MODULE_HPP
