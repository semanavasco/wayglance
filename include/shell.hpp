#pragma once

#include "managers/config.hpp"
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <memory>

namespace wayglance {

/**
 * @class Shell
 * @brief The main application window that serves as a desktop overlay shell.
 *
 * The Shell class creates a transparent overlay window that displays modules
 * in a 3x3 grid layout (top/middle/bottom x left/center/right). It handles:
 * - Monitor-specific window positioning and sizing
 * - Module placement and organization in configurable areas
 * - Overlay transparency and window management
 * - Configuration-driven layout and module loading
 */
class Shell : public Gtk::ApplicationWindow {
public:
  /**
   * @brief Constructs a Shell window for the specified monitor.
   * @param config_manager Shared pointer to the configuration manager.
   * @param monitor The GDK monitor this shell window will be displayed on.
   */
  Shell(std::shared_ptr<wayglance::managers::Config> config_manager, GdkMonitor *monitor);

  /**
   * @brief Destructor for the Shell window.
   */
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
   * @brief Creates and configures the 3x3 grid of module container boxes.
   *
   * Sets up nine boxes arranged in a grid: top-left, top-center, top-right,
   * middle-left, middle-center, middle-right, bottom-left, bottom-center,
   * bottom-right. Each box can contain modules and is positioned according to
   * its alignment.
   */
  void setup_module_boxes();

  /**
   * @brief Configures a single module box with settings from the configuration.
   * @param box The Gtk::Box to configure.
   * @param name The configuration name of the box (e.g., "top-left",
   * "middle-center").
   * @param halign The horizontal alignment for positioning the box in the
   * shell.
   * @param valign The vertical alignment for positioning the box in the shell.
   *
   * Applies configuration settings such as orientation, spacing, margins, and
   * CSS classes based on the box name and configuration file.
   */
  void setup_module_box(Gtk::Box &box, const std::string &name, Gtk::Align halign,
                        Gtk::Align valign);

  /**
   * @brief Loads and instantiates modules into their configured positions.
   *
   * Reads the module configuration to determine which modules should be created
   * and in which container boxes they should be placed. Modules without
   * explicit positioning default to the middle-center container.
   */
  void load_modules();
};

} // namespace wayglance
