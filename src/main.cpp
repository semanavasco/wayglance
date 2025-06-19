#include <cairomm/context.h>
#include <chrono>
#include <clocale>
#include <format>
#include <gdkmm/display.h>
#include <gdkmm/monitor.h>
#include <gtkmm.h>
#include <iostream>
#include <vector>

extern "C" {
#include <gtk4-layer-shell.h>
}

// Declaring a custom window
class WallpaperWindow : public Gtk::ApplicationWindow {
public:
  WallpaperWindow(GdkMonitor *p_monitor);
  virtual ~WallpaperWindow();

protected:
  Gtk::Overlay m_overlay;
  Gtk::DrawingArea m_drawing_area;
  Gtk::Box m_label_box;

  Gtk::Label m_time_label;
  Gtk::Label m_date_label;

  // Methods
  bool update_labels();
  void on_draw(const Cairo::RefPtr<Cairo::Context> &cr, int width, int height);
};

// Window's constructor
WallpaperWindow::WallpaperWindow(GdkMonitor *p_monitor)
    : m_label_box(Gtk::Orientation::VERTICAL, 10) {
  // Window configuration
  set_title("Hyprland Widget");
  set_child(m_overlay);

  // DrawingArea configuration
  // Defining the function that'll be calle dfor redraws
  m_drawing_area.set_draw_func(sigc::mem_fun(*this, &WallpaperWindow::on_draw));

  // Overlay configuration
  m_overlay.set_child(m_drawing_area);
  m_overlay.add_overlay(m_label_box);

  // Configuring label box
  m_label_box.set_valign(Gtk::Align::CENTER);
  m_label_box.set_halign(Gtk::Align::CENTER);
  m_label_box.append(m_time_label);
  m_label_box.append(m_date_label);

  // Configuring layer shell
  gtk_layer_init_for_window((GtkWindow *)gobj());

  // Setting the right monitor
  if (p_monitor)
    gtk_layer_set_monitor((GtkWindow *)gobj(), p_monitor);

  // Configuring layer and anchors
  gtk_layer_set_layer((GtkWindow *)gobj(), GTK_LAYER_SHELL_LAYER_BACKGROUND);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);

  // Disabling keyboard interaction
  gtk_layer_set_keyboard_mode((GtkWindow *)gobj(),
                              GTK_LAYER_SHELL_KEYBOARD_MODE_NONE);

  // Configuring CSS provider and styles
  set_name("wallpaper_window");
  m_time_label.add_css_class("time");
  m_date_label.add_css_class("date");
  auto css_provider = Gtk::CssProvider::create();
  css_provider->load_from_path("style.css");
  Gtk::StyleProvider::add_provider_for_display(
      get_display(), css_provider, GTK_STYLE_PROVIDER_PRIORITY_USER);

  // Configuring timer
  update_labels();
  Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &WallpaperWindow::update_labels), 1000);
}

// Destructor
WallpaperWindow::~WallpaperWindow() {}

// Methods
bool WallpaperWindow::update_labels() {
  const auto now = std::chrono::system_clock::now();
  m_time_label.set_text(std::format("{:%H:%M}", now));
  m_date_label.set_text(std::format("{:%A %d %B %Y}", now));

  return true;
}

void WallpaperWindow::on_draw(const Cairo::RefPtr<Cairo::Context> &cr,
                              int width, int height) {
  // Setting the drawing's color to transparent black
  cr->set_source_rgba(0.0, 0.0, 0.0, 0.0);
  // Painting the wole surface with this transparent color
  cr->paint();
}

// Main entry point
int main(int argc, char *argv[]) {
  // Setting program's locale to the environment one
  std::setlocale(LC_ALL, "");

  // Creating an instance of the GTK app
  auto app = Gtk::Application::create("com.svasco.hyprwidget");

  // Reference to our windows
  std::vector<WallpaperWindow *> windows;

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

    std::cout << "Detected " << n_monitors << " monitors" << std::endl;

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

      // Creating a new instance for our window
      auto window = new WallpaperWindow((GdkMonitor *)monitor->gobj());

      // Adding the window to the app so that it is handled and displayed
      app->add_window(*window);

      // Showing the window
      window->show();

      // Storing the pointer
      windows.push_back(window);
    }
  });

  // Displaying the WallpaperWindow
  return app->run(argc, argv);
}
