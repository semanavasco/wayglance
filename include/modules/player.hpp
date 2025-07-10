#pragma once

#include "module.hpp"

namespace wayglance::modules {

/**
 * @class Player
 * @brief A media player control module that interfaces with MPRIS-compatible
 * media players.
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
   * @param config JSON configuration object containing player settings, icons,
   * and formatting options.
   */
  Player(const nlohmann::json &config);

  /**
   * @brief Destructor for the Player module.
   */
  ~Player();

  /**
   * @enum ConnectionState
   * @brief Represents the current state of the DBus connection to the media
   * player
   */
  enum class ConnectionState {
    Disconnected, ///< No connection established
    Connecting,   ///< Attempting to connect
    Connected,    ///< Successfully connected and operational
    Error         ///< Connection failed or lost
  };

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
  sigc::connection m_prev_connection;
  sigc::connection m_play_pause_connection;
  sigc::connection m_next_connection;
  sigc::connection m_health_check_timer;
  sigc::connection m_retry_timer;
  ConnectionState m_connection_state = ConnectionState::Disconnected;
  std::string m_last_error_message;
  unsigned int m_retry_count = 0;
  static constexpr unsigned int MAX_RETRY_DELAY = 60000;         // 60 seconds
  static constexpr unsigned int HEALTH_CHECK_INTERVAL = 8000;    // 8 seconds
  static constexpr unsigned int PROGRESS_UPDATE_INTERVAL = 1000; // 1 second

  // DBus proxies
  Glib::RefPtr<Gio::DBus::Proxy> m_player_proxy;
  Glib::RefPtr<Gio::DBus::Proxy> m_properties_proxy;

  // Interface names
  static constexpr const char *MPRIS_PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player";
  static constexpr const char *MPRIS_PROPERTIES_INTERFACE = "org.freedesktop.DBus.Properties";
  static constexpr const char *MPRIS_PLAYER_SERVICE = "org.mpris.MediaPlayer2.playerctld";
  static constexpr const char *MPRIS_OBJECT_PATH = "/org/mpris/MediaPlayer2";
  static constexpr const char *MPRIS_BASE_INTERFACE = "org.mpris.MediaPlayer2";

  // Methods
  /**
   * @brief Loads configuration settings for the player module.
   * @param config JSON configuration object containing player name, icons, and
   * display options.
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
  Glib::ustring format_time(gint64 microseconds) const;

  /**
   * @brief Sets the connection state and updates UI accordingly.
   * @param state The new connection state.
   * @param error_message Optional error message for Error state.
   */
  void set_connection_state(ConnectionState state, const std::string &error_message = "");

  /**
   * @brief Gets the current connection state.
   * @return Current connection state.
   */
  ConnectionState get_connection_state() const;

  /**
   * @brief Performs a health check on the DBus connection.
   * @return true if connection is healthy, false otherwise.
   */
  bool check_connection_health();

  /**
   * @brief Starts the periodic health check timer.
   */
  void start_health_check();

  /**
   * @brief Stops the health check timer.
   */
  void stop_health_check();

  /**
   * @brief Updates the UI to reflect the current connection state.
   */
  void update_connection_ui();

  /**
   * @brief Schedules a reconnection attempt with exponential backoff.
   */
  void schedule_reconnection();

  /**
   * @brief Attempts to reconnect to the media player.
   */
  void attempt_reconnection();

  /**
   * @brief Resets the retry logic and counters after successful connection.
   */
  void reset_retry_logic();
};

} // namespace wayglance::modules
