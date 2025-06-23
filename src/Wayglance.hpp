#ifndef DEF_WAYGLANCE_HPP
#define DEF_WAYGLANCE_HPP

#include "managers/ConfigManager.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <memory>

class Wayglance : public Gtk::ApplicationWindow {
public:
  Wayglance(std::shared_ptr<ConfigManager> config_manager,
            GdkMonitor *p_monitor);
  virtual ~Wayglance();

private:
  Gtk::Overlay m_overlay;
  Gtk::DrawingArea m_drawing_area;
  Gtk::Box m_modules_box;

  std::shared_ptr<ConfigManager> m_config_manager;

  // Methods
  void load_modules();
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
};

#endif // !DEF_WAYGLANCE_HPP
