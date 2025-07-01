#include "managers/client.hpp"
#include "managers/config.hpp"
#include <cstdlib>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>
#include <lyra/lyra.hpp>
extern "C" {
#include <gtk4-layer-shell.h>
}

const std::string WAYGLANCE_VERSION = "0.0.32";

int main(int argc, char *argv[]) {
  auto config_manager = std::make_shared<wayglance::managers::Config>();

  // Options values
  bool cli_show_help = false;
  bool cli_show_version = false;
  bool cli_create_defaults = false;
  Glib::ustring cli_config_path;
  Glib::ustring cli_style_path;

  // Options
  auto cli = lyra::cli() | lyra::help(cli_show_help) |
             lyra::opt(cli_show_version)["-v"]["--version"](
                 "Show Wayglance version and exit") |
             lyra::opt(cli_config_path, "path")["-c"]["--config"](
                 "Overrides default config path (default: "
                 "$XDG_CONFIG_HOME/wayglance/config.json or "
                 "$HOME/.config/wayglance/config.json)") |
             lyra::opt(cli_style_path, "path")["-s"]["--style"](
                 "Overrides default style path (default: "
                 "$XDG_CONFIG_HOME/wayglance/style.css or "
                 "$HOME/.config/wayglance/style.css)") |
             lyra::opt(cli_create_defaults)["-d"]["--create-defaults"](
                 "Creates a default configuration at "
                 "$XDG_CONFIG_HOME/wayglance or $HOME/.config/wayglance");

  auto result = cli.parse({argc, argv});
  if (!result) {
    std::cerr << "Error: " << result.message() << std::endl;
    return EXIT_FAILURE;
  }

  if (cli_show_help) {
    std::cout << cli << std::endl;
    return EXIT_SUCCESS;
  }

  if (cli_show_version) {
    std::cout << "Wayglance " << WAYGLANCE_VERSION << std::endl;
    return EXIT_SUCCESS;
  }

  if (cli_create_defaults) {
    if (config_manager->create_defaults())
      return EXIT_SUCCESS;
    else
      return EXIT_FAILURE;
  }

  // Creating app
  auto app =
      Gtk::Application::create("io.github.semanavasco.wayglance",
                               Gio::Application::Flags::HANDLES_COMMAND_LINE);
  wayglance::managers::Client client(app, config_manager);

  // Handling options
  app->signal_command_line().connect(
      [&](const Glib::RefPtr<Gio::ApplicationCommandLine> &cmd) -> int {
        if (!cli_config_path.empty()) {
          if (!config_manager->set_custom_config_path(cli_config_path)) {
            std::cerr << "Error: Couldn't find " << cli_config_path
                      << std::endl;
            app->quit();
            return EXIT_FAILURE;
          }
        }

        if (!cli_style_path.empty()) {
          if (!config_manager->set_custom_style_path(cli_style_path)) {
            std::cerr << "Error: Couldn't find " << cli_style_path << std::endl;
            app->quit();
            return EXIT_FAILURE;
          }
        }

        config_manager->load();

        app->activate();
        return EXIT_SUCCESS;
      },
      false);

  // Starting app
  app->signal_activate().connect([&]() { client.run(); });

  return app->run(argc, argv);
}
