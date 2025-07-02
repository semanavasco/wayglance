#pragma once

#include "module.hpp"

namespace wayglance::modules {

/**
 * @class Player
 * @brief A media player control module that interfaces with MPRIS-compatible media players.
 * 
 * The Player module provides:
 * - Media control buttons (previous, play/pause, next)
 * - Track information display
 * - Progress bar with position and duration
 * - DBus integration with media players via MPRIS protocol
 * - Configurable player selection and icon customization
 * - Support for both regular and Nerd Font icons
 */
class Player : public wayglance::Module {
public:
  /**
   * @brief Constructs a Player module with the given configuration.
   * @param config JSON configuration object containing player settings, icons, and formatting options.
   */
  Player(const nlohmann::json &config);
  
  /**
   * @brief Destructor for the Player module.
   */
  ~Player();

private:
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
  /**
   * @brief Loads configuration settings for the player module.
   * @param config JSON configuration object containing player name, icons, and display options.
   */
  void load_config(const nlohmann::json &config);
  
  /**
   * @brief Establishes DBus connection to the configured media player.
   */
  void get_player_proxy();
  
  /**
   * @brief Handles the previous track button click event.
   */
  void on_prev_clicked();
  
  /**
   * @brief Handles the play/pause button click event.
   */
  void on_play_pause_clicked();
  
  /**
   * @brief Handles the next track button click event.
   */
  void on_next_clicked();
  
  /**
   * @brief Updates all player information (status, metadata, progress).
   */
  void update();
  
  /**
   * @brief Retrieves the current playback status from the media player.
   */
  void get_status();
  
  /**
   * @brief Retrieves the current track metadata from the media player.
   */
  void get_metadata();
  
  /**
   * @brief Retrieves the current playback progress from the media player.
   */
  void get_progress();
  
  /**
   * @brief Updates the UI with current track information and playback status.
   */
  void update_info();
  
  /**
   * @brief Updates the progress bar and position labels.
   * @return true to continue the timer callback, false to stop it.
   */
  bool update_progress();
  
  /**
   * @brief Formats microseconds into a readable time string (MM:SS).
   * @param microseconds Time value in microseconds.
   * @return Formatted time string.
   */
  Glib::ustring format_time(gint64 microseconds);
};
} // namespace wayglance::modules
