#pragma once

#include <gtkmm.h>
#include <nlohmann/json.hpp>

namespace wayglance {

/**
 * @class Module
 * @brief Abstract base class for all display modules in the Wayglance overlay.
 *
 * The Module class provides common functionality for all modules including:
 * - Configuration parsing and application
 * - Alignment and positioning utilities
 * - Standard CSS class and styling support
 */
class Module : public Gtk::Box {
public:
  /**
   * @brief Constructs a Module with the given configuration.
   * @param config JSON configuration object containing module-specific
   * settings.
   *
   * The base constructor handles common configuration options such as
   * alignment, spacing, margins, and orientation that are applicable to all
   * modules.
   */
  Module(const nlohmann::json &config);

  /**
   * @brief Virtual destructor for proper cleanup of derived classes.
   */
  virtual ~Module();

protected:
  /**
   * @brief Converts a string alignment to a Gtk::Align enum value.
   * @param align String representation of alignment ("start", "center", "end",
   * "fill").
   * @return The corresponding Gtk::Align enum value.
   *
   * This utility function helps parse alignment configurations from JSON
   * strings into the appropriate GTK alignment constants for widget
   * positioning.
   */
  Gtk::Align string_to_align(const std::string &align);
};

} // namespace wayglance
