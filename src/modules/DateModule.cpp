#include "DateModule.hpp"
#include <chrono>
#include <format>

// Constructor
DateModule::DateModule() : Gtk::Box(Gtk::Orientation::VERTICAL) {
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

  m_time_label.set_text(std::format("{:%H:%M}", local_time));
  m_date_label.set_text(std::format("{:%A %d %B %Y}", local_time));

  return true;
}
