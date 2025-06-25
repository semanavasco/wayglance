#include "managers/AppManager.hpp"
#include "managers/ConfigManager.hpp"
#include <cairomm/context.h>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>
#include <lyra/lyra.hpp>
extern "C" {
#include <gtk4-layer-shell.h>
}

const std::string WAYGLANCE_VERSION = "0.0.29";

int main(int argc, char *argv[]) {
  // Options values
  bool cli_show_help = false;
  bool cli_show_version = false;
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
                 "$HOME/.config/wayglance/style.css)");

  auto result = cli.parse({argc, argv});
  if (!result) {
    std::cerr << "Error: " << result.message() << std::endl;
    exit(1);
  }

  if (cli_show_help) {
    std::cout << cli << std::endl;
    exit(0);
  }

  if (cli_show_version) {
    std::cout << "Wayglance " << WAYGLANCE_VERSION << std::endl;
    exit(0);
  }

  // Creating managers
  auto app =
      Gtk::Application::create("io.github.semanavasco.wayglance",
                               Gio::Application::Flags::HANDLES_COMMAND_LINE);
  auto config_manager = std::make_shared<ConfigManager>();
  AppManager app_manager(app, config_manager);

  // Handling options
  app->signal_command_line().connect(
      [&](const Glib::RefPtr<Gio::ApplicationCommandLine> &cmd) -> int {
        if (!cli_config_path.empty())
          config_manager->set_custom_config_path(cli_config_path);

        if (!cli_style_path.empty())
          config_manager->set_custom_style_path(cli_style_path);

        config_manager->setup_paths();
        config_manager->load_config();
        config_manager->load_style();

        app->activate();
        return 0;
      },
      false);

  // Starting app
  app->signal_activate().connect([&]() { app_manager.run(); });

  return app->run(argc, argv);
}
