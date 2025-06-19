#ifndef DEF_WAYGLANCE_HPP
#define DEF_WAYGLANCE_HPP

#include "modules/DateModule.hpp"
#include "vendor/json.hpp"
#include <filesystem>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>

class Wayglance : public Gtk::ApplicationWindow {
public:
  Wayglance(GdkMonitor *p_monitor);
  virtual ~Wayglance();

protected:
  Gtk::Overlay m_overlay;
  Gtk::DrawingArea m_drawing_area;
  Gtk::Box m_modules_box;

  // Configuration
  std::filesystem::path m_config_dir_path;
  nlohmann::json m_config;

  void setup_paths();
  void load_style();
  void load_config();
  void load_modules();

  // General Methods
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
};

#endif // !DEF_WAYGLANCE_HPP
