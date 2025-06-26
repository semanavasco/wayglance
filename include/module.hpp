#pragma once

#include <gtkmm.h>
#include <nlohmann/json.hpp>

namespace wayglance {

/**
 * @class Module
 * @brief The base class for module creation. Handles positioning, alignment,
 * spacing, etc.
 */
class Module : public Gtk::Box {
public:
  Module(const nlohmann::json &config);
  virtual ~Module();

protected:
  /**
   * @brief Converts a string alignment to a Gtk::Align enum value.
   * @param align The string to convert to a Gtk::Align.
   */
  Gtk::Align string_to_align(const std::string &align);
};

} // namespace wayglance
