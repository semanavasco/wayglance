#include "Wayglance.hpp"
#include <cairomm/context.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>

extern "C" {
#include <gtk4-layer-shell.h>
}

// Main entry point
int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("io.github.semanavasco.wayglance");

  // Connecting to the activate signal on startup
  app->signal_activate().connect([&]() {
    // Getting the default display
    auto display = Gdk::Display::get_default();
    if (!display)
      return;

    // Getting a list of all monitors
    auto monitors = display->get_monitors();
    if (!monitors)
      return;

    // Getting the number of monitors via the C function
    guint n_monitors = g_list_model_get_n_items(monitors->gobj());
    std::cout << "Detected " << n_monitors << " monitor(s)" << std::endl;

    for (guint i = 0; i < n_monitors; i++) {
      // Getting the raw C object
      void *monitor_gobject = g_list_model_get_item(monitors->gobj(), i);
      if (!monitor_gobject)
        continue;

      // Wrapping the C-Object in a C++ Glib::RefPtr to manipulate it
      auto monitor_wrapper = Glib::wrap((GObject *)monitor_gobject);

      // Ensuring it's a monitor
      auto monitor = std::dynamic_pointer_cast<Gdk::Monitor>(monitor_wrapper);
      if (!monitor)
        continue;

      std::cout << "Creating widget for monitor n°" << i << std::endl;

      // Creating a new instance
      auto window = new Wayglance((GdkMonitor *)monitor->gobj());
      app->add_window(*window);
      window->show();
    }
  });

  // Displaying the WallpaperWindow
  return app->run(argc, argv);
}
