#include "modules/player.hpp"
#include <spdlog/spdlog.h>

using namespace wayglance;

// Constructor
modules::Player::Player(const nlohmann::json &config) : Module(config) {
  load_config(config);

  // Widgets
  m_track_label.set_text("Nothing's playing currently...");

  m_button_box.set_orientation(Gtk::Orientation::HORIZONTAL);
  m_button_box.set_halign(Gtk::Align::CENTER);
  m_button_box.set_spacing(10);

  // Buttons icons
  if (m_use_nerd_font) {
    m_prev_button.set_label(m_icon_prev);
    m_play_pause_button.set_label(m_icon_play);
    m_next_button.set_label(m_icon_next);
  } else {
    m_prev_button.set_icon_name(m_icon_prev);
    m_play_pause_button.set_icon_name(m_icon_play);
    m_next_button.set_icon_name(m_icon_next);
  }

  // Progress box
  m_progress_box.set_orientation(Gtk::Orientation::HORIZONTAL);
  m_progress_box.set_halign(Gtk::Align::CENTER);
  m_progress_box.set_spacing(10);

  m_position_label.set_valign(Gtk::Align::CENTER);
  m_progress_bar.set_valign(Gtk::Align::CENTER);
  m_duration_label.set_valign(Gtk::Align::CENTER);

  // CSS classes and IDs
  set_name("module-player");

  m_track_label.set_name("player-track-label");
  m_track_label.add_css_class("player-labels");

  m_button_box.set_name("player-button-box");
  m_prev_button.add_css_class("player-buttons");
  m_prev_button.set_name("player-previous-button");
  m_play_pause_button.add_css_class("player-buttons");
  m_play_pause_button.set_name("player-play-pause-button");
  m_next_button.add_css_class("player-buttons");
  m_next_button.set_name("player-next-button");

  m_progress_box.set_name("player-progress-box");
  m_position_label.set_name("player-position-label");
  m_position_label.add_css_class("player-labels");
  m_duration_label.set_name("player-duration-label");
  m_duration_label.add_css_class("player-labels");
  m_progress_bar.set_name("player-progress-bar");

  // Adding widgets
  m_button_box.append(m_prev_button);
  m_button_box.append(m_play_pause_button);
  m_button_box.append(m_next_button);

  m_progress_box.append(m_position_label);
  m_progress_box.append(m_progress_bar);
  m_progress_box.append(m_duration_label);

  append(m_track_label);
  append(m_progress_box);
  append(m_button_box);

  // Registering event handlers
  m_prev_connection = m_prev_button.signal_clicked().connect(
      sigc::mem_fun(*this, &Player::on_prev_clicked));
  m_play_pause_connection = m_play_pause_button.signal_clicked().connect(
      sigc::mem_fun(*this, &Player::on_play_pause_clicked));
  m_next_connection = m_next_button.signal_clicked().connect(
      sigc::mem_fun(*this, &Player::on_next_clicked));

  // Setting up proxy
  get_player_proxy();
}

// Destructor
modules::Player::~Player() {
  // Stop health check timer
  if (m_health_check_timer.connected())
    m_health_check_timer.disconnect();

  if (m_progress_timeout.connected())
    m_progress_timeout.disconnect();

  if (m_prev_connection.connected())
    m_prev_connection.disconnect();

  if (m_play_pause_connection.connected())
    m_play_pause_connection.disconnect();

  if (m_next_connection.connected())
    m_next_connection.disconnect();

  if (m_retry_timer.connected())
    m_retry_timer.disconnect();
}

// Methods
void modules::Player::load_config(const nlohmann::json &config) {
  // Load player name configuration
  auto player_name_str = config.value("player", "any");
  if (player_name_str == "any")
    m_player_name = "";
  else {
    // Validate player name format
    if (player_name_str.empty()) {
      spdlog::warn(
          "Player: Empty player name provided, using automatic detection");
      m_player_name = "";
    } else {
      m_player_name = "org.mpris.MediaPlayer2." + player_name_str;
      spdlog::debug("Player: Using specific player: {}",
                    std::string(m_player_name));
    }
  }

  // Load font configuration
  m_use_nerd_font = config.value("nerd-font", false);

  // Load button configurations with defaults
  const nlohmann::json buttons_config =
      config.value("buttons", nlohmann::json::object());

  const auto &btn_prev_cfg =
      buttons_config.value("previous", nlohmann::json::object());
  m_icon_prev = btn_prev_cfg.value("icon", "media-skip-backward-symbolic");

  const auto &btn_next_cfg =
      buttons_config.value("next", nlohmann::json::object());
  m_icon_next = btn_next_cfg.value("icon", "media-skip-forward-symbolic");

  const auto &btn_play_cfg =
      buttons_config.value("play", nlohmann::json::object());
  m_icon_play = btn_play_cfg.value("icon", "media-playback-start-symbolic");

  const auto &btn_pause_cfg =
      buttons_config.value("pause", nlohmann::json::object());
  m_icon_pause = btn_pause_cfg.value("icon", "media-playback-pause-symbolic");
}

void modules::Player::get_player_proxy() {
  // Set connecting state
  set_connection_state(ConnectionState::Connecting);

  // Service name, object path and MPRIS interfaces
  const Glib::ustring name = MPRIS_PLAYER_SERVICE;
  const Glib::ustring path = MPRIS_OBJECT_PATH;
  const Glib::ustring player_interface = MPRIS_PLAYER_INTERFACE;
  const Glib::ustring properties_interface = MPRIS_PROPERTIES_INTERFACE;

  try {
    // Creating player interface proxy
    Gio::DBus::Proxy::create_for_bus(
        Gio::DBus::BusType::SESSION, name, path, player_interface,
        [=, this](const Glib::RefPtr<Gio::AsyncResult> &result) {
          m_player_proxy = Gio::DBus::Proxy::create_for_bus_finish(result);

          if (!m_player_proxy) {
            set_connection_state(ConnectionState::Error,
                                 "Failed to create player proxy");
            schedule_reconnection();
            return;
          }
          spdlog::debug("Player: Successfully connected to media player");

          try {
            auto bus =
                Gio::DBus::Connection::get_sync(Gio::DBus::BusType::SESSION);
            if (bus) {
              bus->signal_subscribe(
                  [=,
                   this](const Glib::RefPtr<Gio::DBus::Connection> &connection,
                         const Glib::ustring &sender_name,
                         const Glib::ustring &object_path,
                         const Glib::ustring &interface_name,
                         const Glib::ustring &signal_name,
                         const Glib::VariantContainerBase &parameters) {
                    if (interface_name == properties_interface &&
                        signal_name == "PropertiesChanged")
                      update();
                  },
                  m_player_name, properties_interface, "PropertiesChanged",
                  path);
            }
          } catch (const Glib::Error &e) {
            spdlog::error("Player: Couldn't subscribe to DBus "
                          "PropertiesChanged signal : {}",
                          e.what());
            set_connection_state(
                ConnectionState::Error,
                std::string("Failed to subscribe to signals: ") + e.what());
            return;
          }

          // Set connected state and perform initial update
          set_connection_state(ConnectionState::Connected);
          update();
        });

    // Creating properties interface proxy
    m_properties_proxy = Gio::DBus::Proxy::create_for_bus_sync(
        Gio::DBus::BusType::SESSION, name, path, properties_interface);
  } catch (const Glib::Error &e) {
    spdlog::error("Player: Couldn't create DBus proxies : {}", e.what());
    m_player_proxy.reset();
    m_properties_proxy.reset();
    set_connection_state(ConnectionState::Error,
                         std::string("Failed to create proxies: ") + e.what());
    schedule_reconnection();
  }
}

void modules::Player::on_prev_clicked() {
  try {
    if (m_player_proxy)
      m_player_proxy->call("Previous");
  } catch (const std::exception &e) {
    spdlog::error("Player: Couldn't jump to previous track : {}", e.what());
  }
}

void modules::Player::on_play_pause_clicked() {
  try {
    if (m_player_proxy)
      m_player_proxy->call("PlayPause");
  } catch (const std::exception &e) {
    spdlog::error("Player: Couldn't toggle play/pause on track : {}", e.what());
  }
}

void modules::Player::on_next_clicked() {
  try {
    if (m_player_proxy)
      m_player_proxy->call("Next");
  } catch (const std::exception &e) {
    spdlog::error("Player: Couldn't jump to next track : {}", e.what());
  }
}

void modules::Player::update() {
  // Check connection state first
  if (m_connection_state != ConnectionState::Connected) {
    // If not connected, update UI to show disconnected state
    update_connection_ui();
    return;
  }

  // Set defaults if proxys weren't created
  if (!m_player_proxy || !m_properties_proxy) {
    spdlog::debug("Player: Proxies not available, attempting reconnection");
    set_connection_state(ConnectionState::Error, "Proxies are null");
    return;
  }

  // Update status and metadata
  get_status();
  get_metadata();
  get_progress();
  update_info();
}

void modules::Player::get_status() {
  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring(MPRIS_PLAYER_INTERFACE)),
         Glib::create_variant(Glib::ustring("PlaybackStatus"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters, 2000);

    Glib::Variant<Glib::VariantBase> property_value_variant;
    call_result.get_child(property_value_variant, 0);

    auto status_variant_string =
        Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
            property_value_variant.get());

    if (status_variant_string) {
      m_status = status_variant_string.get();
      if (m_status == "Playing") {
        m_playing = true;
        m_paused = false;
      } else if (m_status == "Paused") {
        m_playing = false;
        m_paused = true;
      } else {
        m_playing = false;
        m_paused = false;
      }
    }
  } catch (const Glib::Error &e) {
    spdlog::error("Player: Couldn't get status : {}", e.what());
    m_status = "Unknown";
    m_playing = false;
    m_paused = false;
    set_connection_state(ConnectionState::Error,
                         "Failed to get status: " + std::string(e.what()));
  } catch (const std::exception &e) {
    spdlog::error("Player: Unexpected error getting status: {}", e.what());
    set_connection_state(ConnectionState::Error,
                         "Unexpected error: " + std::string(e.what()));
  }
}

void modules::Player::get_metadata() {
  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring(MPRIS_PLAYER_INTERFACE)),
         Glib::create_variant(Glib::ustring("Metadata"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters, 2000);

    Glib::Variant<Glib::VariantBase> property_value_variant;
    call_result.get_child(property_value_variant, 0);

    auto dict_variant = Glib::VariantBase::cast_dynamic<
        Glib::Variant<std::map<Glib::ustring, Glib::VariantBase>>>(
        property_value_variant.get());

    if (!dict_variant) {
      m_track = "Nothing's playing currently...";
      m_duration = 0;
      return;
    }

    auto metadata_map = dict_variant.get();

    // Reset track string
    m_track.clear();

    // Getting the title
    auto title_it = metadata_map.find("xesam:title");
    if (title_it != metadata_map.end()) {
      auto title_variant =
          Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(
              title_it->second);
      if (title_variant)
        m_track = title_variant.get();
    }

    // Getting the artist(s)
    auto artist_it = metadata_map.find("xesam:artist");
    if (artist_it != metadata_map.end()) {
      auto artist_variant = Glib::VariantBase::cast_dynamic<
          Glib::Variant<std::vector<Glib::ustring>>>(artist_it->second);
      if (artist_variant) {
        auto artists = artist_variant.get();

        if (!artists.empty()) {
          if (!m_track.empty())
            m_track += " - ";

          for (int i = 0; i < artists.size(); i++) {
            m_track += artists[i];
            if (i < artists.size() - 1)
              m_track += ", ";
          }
        }
      }
    }

    // Set default if no track info found
    if (m_track.empty())
      m_track = "Nothing's playing currently...";

    // Getting track duration
    auto duration_it = metadata_map.find("mpris:length");
    if (duration_it != metadata_map.end()) {
      auto duration_variant =
          Glib::VariantBase::cast_dynamic<Glib::Variant<guint64>>(
              duration_it->second);

      if (duration_variant)
        m_duration = static_cast<gint64>(duration_variant.get());
    }
  } catch (const Glib::Error &e) {
    spdlog::error("Player: Couldn't get metadata : {}", e.what());
    m_track = "Nothing's playing currently...";
    m_duration = 0;
    set_connection_state(ConnectionState::Error,
                         "Failed to get metadata: " + std::string(e.what()));
  } catch (const std::exception &e) {
    spdlog::error("Player: Unexpected error getting metadata: {}", e.what());
    m_track = "Nothing's playing currently...";
    m_duration = 0;
    set_connection_state(ConnectionState::Error,
                         "Unexpected error: " + std::string(e.what()));
  }
}

Glib::ustring modules::Player::format_time(gint64 microseconds) const {
  if (microseconds == 0)
    return "0:00";

  gint64 seconds = microseconds / 1000000;
  gint64 minutes = seconds / 60;
  seconds %= 60;

  char buffer[6];
  std::snprintf(buffer, sizeof(buffer), "%ld:%02ld", minutes, seconds);
  return buffer;
}

void modules::Player::get_progress() {
  if (!m_properties_proxy)
    return;

  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring(MPRIS_PLAYER_INTERFACE)),
         Glib::create_variant(Glib::ustring("Position"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters, 1000);

    Glib::Variant<Glib::VariantBase> property_value_variant;
    call_result.get_child(property_value_variant, 0);

    auto progress_variant =
        Glib::VariantBase::cast_dynamic<Glib::Variant<gint64>>(
            property_value_variant.get());

    if (progress_variant)
      m_position = progress_variant.get();
    else
      m_position = 0;
  } catch (const Glib::Error &e) {
    spdlog::debug("Player: Couldn't get progress: {}", e.what());
    m_position = 0;
  } catch (const std::exception &e) {
    spdlog::debug("Player: Unexpected error getting progress: {}", e.what());
    m_position = 0;
  }
}

bool modules::Player::update_progress() {
  if (!m_playing)
    return false;

  // Asking for progress
  get_progress();

  // Updating display
  m_position_label.set_text(format_time(m_position));
  if (m_duration > 0) {
    double fraction = static_cast<double>(m_position) / m_duration;
    m_progress_bar.set_fraction(fraction);
  } else
    m_progress_bar.set_fraction(0.0);

  return true;
}

void modules::Player::update_info() {
  if (m_status == "Playing") {
    if (m_use_nerd_font)
      m_play_pause_button.set_label(m_icon_pause);
    else
      m_play_pause_button.set_icon_name(m_icon_pause);

    // Start the progress timer
    if (!m_progress_timeout.connected())
      m_progress_timeout = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &Player::update_progress),
          PROGRESS_UPDATE_INTERVAL);
  } else {
    if (m_use_nerd_font)
      m_play_pause_button.set_label(m_icon_play);
    else
      m_play_pause_button.set_icon_name(m_icon_play);

    // Stop the progress timer
    if (m_progress_timeout.connected())
      m_progress_timeout.disconnect();
  }

  if (!m_track.empty())
    m_track_label.set_text(m_track);
  else
    m_track_label.set_text("Nothing's playing currently...");

  // Update the progress infos
  m_position_label.set_text(format_time(m_position));
  m_duration_label.set_text(format_time(m_duration));

  if (m_duration > 0) {
    double fraction = static_cast<double>(m_position) / m_duration;
    m_progress_bar.set_fraction(fraction);
  } else
    m_progress_bar.set_fraction(0.0);
}

void modules::Player::set_connection_state(
    modules::Player::ConnectionState state, const std::string &error_message) {
  // Only update if state actually changed
  if (m_connection_state == state && error_message == m_last_error_message)
    return;

  m_connection_state = state;
  m_last_error_message = error_message;

  // Log state transitions
  switch (state) {
  case ConnectionState::Disconnected:
    spdlog::debug("Player: Connection state changed to Disconnected");
    stop_health_check();
    break;
  case ConnectionState::Connecting:
    spdlog::debug("Player: Connection state changed to Connecting");
    break;
  case ConnectionState::Connected:
    spdlog::debug("Player: Connection state changed to Connected");
    start_health_check();
    reset_retry_logic();
    break;
  case ConnectionState::Error:
    spdlog::error("Player: Connection state changed to Error - {}",
                  error_message);
    stop_health_check();
    schedule_reconnection();
    break;
  }

  // Update UI to reflect connection state
  update_connection_ui();
}

modules::Player::ConnectionState modules::Player::get_connection_state() const {
  return m_connection_state;
}

bool modules::Player::check_connection_health() {
  if (!m_properties_proxy) {
    set_connection_state(ConnectionState::Error, "Properties proxy is null");
    schedule_reconnection();
    return false;
  }

  try {
    // Perform a DBus call to test connection health
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring(MPRIS_BASE_INTERFACE)),
         Glib::create_variant(Glib::ustring("Identity"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters, 1000);

    // If we get here, the connection is healthy
    if (m_connection_state != ConnectionState::Connected)
      set_connection_state(ConnectionState::Connected);
    return true;
  } catch (const Glib::Error &e) {
    spdlog::error("Player: Health check failed: {}", e.what());
    set_connection_state(ConnectionState::Error,
                         std::string("Health check failed: ") + e.what());
    return false;
  }
}

void modules::Player::start_health_check() {
  // Don't start if already running
  if (m_health_check_timer.connected())
    return;

  spdlog::debug("Player: Starting health check timer");
  m_health_check_timer = Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &Player::check_connection_health),
      HEALTH_CHECK_INTERVAL);
}

void modules::Player::stop_health_check() {
  if (!m_health_check_timer.connected())
    return;

  spdlog::debug("Player: Stopping health check timer");
  m_health_check_timer.disconnect();
}

void modules::Player::update_connection_ui() {
  std::string connection_status;

  switch (m_connection_state) {
  case ConnectionState::Disconnected:
    connection_status = "Media Player Disconnected";
    // Disable control buttons
    m_prev_button.set_sensitive(false);
    m_play_pause_button.set_sensitive(false);
    m_next_button.set_sensitive(false);
    break;

  case ConnectionState::Connecting:
    connection_status = "Connecting to Media Player...";
    // Disable control buttons during connection
    m_prev_button.set_sensitive(false);
    m_play_pause_button.set_sensitive(false);
    m_next_button.set_sensitive(false);
    break;

  case ConnectionState::Connected:
    // Don't show connection status when connected, show actual track info
    connection_status = "";

    // Enable control buttons
    m_prev_button.set_sensitive(true);
    m_play_pause_button.set_sensitive(true);
    m_next_button.set_sensitive(true);
    break;

  case ConnectionState::Error:
    connection_status = "Media Player Error, retrying...";
    // Disable control buttons on error
    m_prev_button.set_sensitive(false);
    m_play_pause_button.set_sensitive(false);
    m_next_button.set_sensitive(false);
    break;
  }

  // Update track label with connection status when not connected
  if (m_connection_state != ConnectionState::Connected) {
    m_track_label.set_text(connection_status);

    // Clear progress information when disconnected
    m_position_label.set_text("0:00");
    m_duration_label.set_text("0:00");
    m_progress_bar.set_fraction(0.0);
  }
}

void modules::Player::schedule_reconnection() {
  // Don't schedule if already scheduled
  if (m_retry_timer.connected())
    return;

  // Calculate the delay
  unsigned int delay = std::min(1000u << m_retry_count, MAX_RETRY_DELAY);

  spdlog::debug("Player: Scheduling reconnection attempt {} in {}ms",
                m_retry_count + 1, delay);

  m_retry_timer = Glib::signal_timeout().connect(
      [this]() {
        attempt_reconnection();
        return false;
      },
      delay);
}

void modules::Player::attempt_reconnection() {
  m_retry_count++;
  spdlog::debug("Player: Attempting reconnection #{}", m_retry_count);

  // Remove existing timer
  if (m_retry_timer.connected())
    m_retry_timer.disconnect();

  // Try to connect
  get_player_proxy();
}

void modules::Player::reset_retry_logic() {
  m_retry_count = 0;

  if (m_retry_timer.connected())
    m_retry_timer.disconnect();
}
