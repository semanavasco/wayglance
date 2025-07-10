#include "module.hpp"
#include <spdlog/spdlog.h>

// Constructor
wayglance::Module::Module(const nlohmann::json &config) : Gtk::Box() {
  // Read orientation from config
  set_orientation(string_to_orientation(config.value("orientation", "vertical")));

  // Read alignment from config
  set_halign(string_to_align(config.value("h-align", "center")));
  set_valign(string_to_align(config.value("v-align", "center")));

  // Read spacing from config
  set_spacing(config.value("spacing", 0));
}

// Destructor
wayglance::Module::~Module() {}

// Methods
Gtk::Orientation wayglance::Module::string_to_orientation(const std::string &orientation) {
  if (orientation == "horizontal")
    return Gtk::Orientation::HORIZONTAL;

  if (orientation != "vertical")
    spdlog::warn("Incorrect orientation, falling back to \"vertical\"");

  return Gtk::Orientation::VERTICAL;
}

Gtk::Align wayglance::Module::string_to_align(const std::string &align) {
  if (align == "start")
    return Gtk::Align::START;

  if (align == "end")
    return Gtk::Align::END;

  if (align == "fill")
    return Gtk::Align::FILL;

  if (align != "center")
    spdlog::warn("Incorrect alignment, falling back to \"center\"");

  return Gtk::Align::CENTER;
}
