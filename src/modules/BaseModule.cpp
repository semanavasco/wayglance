#include "BaseModule.hpp"

// Constructor
BaseModule::BaseModule(const nlohmann::json &config) : Gtk::Box() {
  // Read orientation from config
  std::string orientation = config.value("orientation", "vertical");
  set_orientation(orientation == "horizontal" ? Gtk::Orientation::HORIZONTAL
                                              : Gtk::Orientation::VERTICAL);

  // Read alignment from config
  set_halign(string_to_align(config.value("h-align", "center")));
  set_valign(string_to_align(config.value("v-align", "center")));

  // Read spacing from config
  set_spacing(config.value("spacing", 0));
}

// Destructor
BaseModule::~BaseModule() {}

// Methods
Gtk::Align BaseModule::string_to_align(const std::string &align) {
  if (align == "start")
    return Gtk::Align::START;
  if (align == "end")
    return Gtk::Align::END;
  if (align == "fill")
    return Gtk::Align::FILL;
  if (align == "center")
    return Gtk::Align::CENTER;
  return Gtk::Align::CENTER;
}
