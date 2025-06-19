#include "DateModule.hpp"
#include <chrono>
#include <format>

// Constructor
DateModule::DateModule(const nlohmann::json &config)
    : Gtk::Box(Gtk::Orientation::VERTICAL) {
  // Reading config with default fallback
  m_time_format = std::format("{{:{}}}", config.value("time_format", "%H:%M"));
  m_date_format =
      std::format("{{:{}}}", config.value("date_format", "%A %d %B %Y"));

  // Configuring module
  set_valign(Gtk::Align::CENTER);
  set_halign(Gtk::Align::CENTER);
  append(m_time_label);
  append(m_date_label);

  m_time_label.add_css_class("time");
  m_date_label.add_css_class("date");

  // Configuring timer
  update_labels();
  Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &DateModule::update_labels), 1000);
}

// Destructor
DateModule::~DateModule() {}

// Methods
bool DateModule::update_labels() {
  const auto now = std::chrono::system_clock::now();
  const std::chrono::zoned_time local_time{std::chrono::current_zone(), now};

  m_time_label.set_text(
      std::vformat(m_time_format, std::make_format_args(local_time)));
  m_date_label.set_text(
      std::vformat(m_date_format, std::make_format_args(local_time)));

  return true;
}
