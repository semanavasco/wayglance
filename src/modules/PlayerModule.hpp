#ifndef PLAYER_MODULE_HPP
#define PLAYER_MODULE_HPP

#include "../vendor/json.hpp"
#include <giomm.h>
#include <glibmm.h>
#include <gtkmm.h>

class PlayerModule : public Gtk::Box {
public:
  PlayerModule(const nlohmann::json &config);
  ~PlayerModule();

protected:
  // Widgets
  Gtk::Label m_track_label;

  Gtk::Box m_button_box;
  Gtk::Button m_prev_button;
  Gtk::Button m_play_pause_button;
  Gtk::Button m_next_button;

  Gtk::Box m_progress_box;
  Gtk::Label m_position_label;
  Gtk::ProgressBar m_progress_bar;
  Gtk::Label m_duration_label;

  // Configuration
  Glib::ustring m_player_name;
  bool m_use_nerd_font = false;
  Glib::ustring m_icon_prev;
  Glib::ustring m_icon_next;
  Glib::ustring m_icon_play;
  Glib::ustring m_icon_pause;

  // State
  bool m_playing = false;
  bool m_paused = false;
  Glib::ustring m_track;
  Glib::ustring m_status;
  gint64 m_position = 0;
  gint64 m_duration = 0;
  sigc::connection m_progress_timeout;

  // DBus proxys
  Glib::RefPtr<Gio::DBus::Proxy> m_player_proxy;
  Glib::RefPtr<Gio::DBus::Proxy> m_properties_proxy;

  // Methods
  void load_config(const nlohmann::json &config);
  void get_player_proxy();
  void on_prev_clicked();
  void on_play_pause_clicked();
  void on_next_clicked();
  void update();
  void get_status();
  void get_metadata();
  void get_progress();
  void update_info();
  bool update_progress();
  Glib::ustring format_time(gint64 microseconds);
};

#endif // PLAYER_MODULE_HPP
