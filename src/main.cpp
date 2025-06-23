#include "managers/AppManager.hpp"
#include "managers/ConfigManager.hpp"
#include <cairomm/context.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>

extern "C" {
#include <gtk4-layer-shell.h>
}

int main(int argc, char *argv[]) {
  auto app = Gtk::Application::create("io.github.semanavasco.wayglance");

  auto config_manager = std::make_shared<ConfigManager>();
  AppManager app_manager(app, config_manager);

  app->signal_activate().connect([&]() { app_manager.run(); });

  return app->run(argc, argv);
}
