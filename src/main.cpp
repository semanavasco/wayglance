#include <cairomm/context.h>
#include <chrono>
#include <clocale>
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
WallpaperWindow::WallpaperWindow()
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
  gtk_layer_set_layer((GtkWindow *)gobj(), GTK_LAYER_SHELL_LAYER_BACKGROUND);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
  gtk_layer_set_anchor((GtkWindow *)gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);
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
  auto app = Gtk::Application::create("org.example.hyprwidget");

  // Displaying the WallpaperWindow
  return app->make_window_and_run<WallpaperWindow>(argc, argv);
}
