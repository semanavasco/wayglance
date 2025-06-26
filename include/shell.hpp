#pragma once

#include "managers/config.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <memory>

namespace wayglance {

/**
 * @class Shell
 * @brief The background shell.
 */
class Shell : public Gtk::ApplicationWindow {
public:
  Shell(std::shared_ptr<wayglance::managers::Config> config_manager,
        GdkMonitor *p_monitor);
  virtual ~Shell();

private:
  Gtk::Overlay m_overlay;

  // Modules Areas
  Gtk::Box m_top_left_box, m_top_center_box, m_top_right_box;
  Gtk::Box m_middle_left_box, m_middle_center_box, m_middle_right_box;
  Gtk::Box m_bottom_left_box, m_bottom_center_box, m_bottom_right_box;

  // Configuration
  std::shared_ptr<wayglance::managers::Config> m_config_manager;

  // Methods
  /**
   * @brief Creates the different modules boxes : top-left, top-center,
   * top-right, middle... bottom...
   */
  void setup_module_boxes();

  /**
   * @brief Sets up a single module box, loads its orientation, spacing,  name,
   * etc, from the configuration file if existing.
   * @param box The Gtk::Box to configure.
   * @param name The name of the box to configure (top-left, ...)
   * @param halign The halign to position itself in the shell object.
   * @param valign The halign to position itself in the shell object.
   */
  void setup_module_box(Gtk::Box &box, const std::string &name,
                        Gtk::Align halign, Gtk::Align valign);

  /**
   * @brief Loads the different modules into their respective positions (if
   * set).
   * * Defaults their position to middle-center if no configuration is given.
   */
  void load_modules();
};

} // namespace wayglance
