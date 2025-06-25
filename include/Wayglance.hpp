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

  // Modules Areas
  Gtk::Box m_top_left_box, m_top_center_box, m_top_right_box;
  Gtk::Box m_middle_left_box, m_middle_center_box, m_middle_right_box;
  Gtk::Box m_bottom_left_box, m_bottom_center_box, m_bottom_right_box;

  // Configuration
  std::shared_ptr<ConfigManager> m_config_manager;

  // Methods
  void setup_module_boxes();
  void setup_module_box(Gtk::Box &box, const std::string &name,
                        Gtk::Align halign, Gtk::Align valign);
  void load_modules();
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
};

#endif // !DEF_WAYGLANCE_HPP
