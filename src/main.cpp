#include <chrono>
#include <format>
#include <gtkmm.h>
extern "C" {
#include <gtk4-layer-shell.h>
}

// Declaring a custom window
class WallpaperWindow : public Gtk::ApplicationWindow {
public:
  WallpaperWindow();
  virtual ~WallpaperWindow();

protected:
  Gtk::Box m_main_box;
  Gtk::Label m_time_label;
  Gtk::Label m_date_label;

  bool update_labels();
};

// Window's constructor
WallpaperWindow::WallpaperWindow()
    : m_main_box(Gtk::Orientation::VERTICAL, 10) {
  // Layer shell configuration
  gtk_layer_init_for_window((GtkWindow *)gobj());

  // Setting the layer as background
  gtk_layer_set_layer((GtkWindow *)gobj(), GTK_LAYER_SHELL_LAYER_BACKGROUND);

  // Anchoring the window to the 4 corners of the screen
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);

  // Deactivate keyboard interactivity
  gtk_layer_set_keyboard_mode((GtkWindow *)gobj(),
                              GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

  // Initial window configuration
  set_title("Hyprland Widget");
  set_child(m_main_box); // Adding the main box to the window

  // Configuring the main window to center its content
  m_main_box.set_valign(Gtk::Align::CENTER);
  m_main_box.set_halign(Gtk::Align::CENTER);

  // Adding labels to the main box
  m_main_box.append(m_time_label);
  m_main_box.append(m_date_label);

  // Adding css classes to the labels
  m_time_label.add_css_class("time");
  m_date_label.add_css_class("date");

  // Creating a CSS loader
  auto css_provider = Gtk::CssProvider::create();
  // Loading the file from a specified path
  css_provider->load_from_path("style.css");
  // Applying the css provider to the whole display
  Gtk::StyleProvider::add_provider_for_display(
      get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  // Configuring the timer
  update_labels();

  // Connecting update_labels to a signal that triggers every 1000ms
  Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &WallpaperWindow::update_labels), 1000);
}

// Destructor
WallpaperWindow::~WallpaperWindow() {}

// Label update method
bool WallpaperWindow::update_labels() {
  // Getting the current timestamp
  const auto now = std::chrono::system_clock::now();

  // Formatting and updating the hour label
  m_time_label.set_text(std::format("{:%H:%M}", now));

  // Formatting and updating the date label
  m_date_label.set_text(std::format("{:%A %d %B %Y}", now));

  // Returns 'true' so that the timer continues running
  return true;
}

// Main entry point
int main(int argc, char *argv[]) {
  // Creating an instance of the GTK app
  auto app = Gtk::Application::create("org.example.hyprwidget");

  // Displaying the WallpaperWindow
  return app->make_window_and_run<WallpaperWindow>(argc, argv);
}
