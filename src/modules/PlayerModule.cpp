#include "PlayerModule.hpp"
#include "../vendor/json.hpp"
#include "glibmm/main.h"
#include <cstdio>
#include <iostream>
#include <map>
#include <vector>

// Constructor
PlayerModule::PlayerModule(const nlohmann::json &config)
    : Gtk::Box(Gtk::Orientation::VERTICAL) {
  // Reading config with default fallback
  m_player_name = config.value("player", "any");
  m_player_name == "any"
      ? m_player_name = ""
      : m_player_name = "org.mpris.MediaPlayer2." + m_player_name;

  m_use_nerd_font = config.value("nerd-font", false);

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
  m_prev_button.signal_clicked().connect(
      sigc::mem_fun(*this, &PlayerModule::on_prev_clicked));
  m_play_pause_button.signal_clicked().connect(
      sigc::mem_fun(*this, &PlayerModule::on_play_pause_clicked));
  m_next_button.signal_clicked().connect(
      sigc::mem_fun(*this, &PlayerModule::on_next_clicked));

  // Setting up proxy
  get_player_proxy();
}

// Destructor
PlayerModule::~PlayerModule() {}

// Methods
void PlayerModule::get_player_proxy() {
  // Service name, object path and MPRIS interfaces
  const Glib::ustring name = "org.mpris.MediaPlayer2.playerctld";
  const Glib::ustring path = "/org/mpris/MediaPlayer2";
  const Glib::ustring player_interface = "org.mpris.MediaPlayer2.Player";
  const Glib::ustring properties_interface = "org.freedesktop.DBus.Properties";

  try {
    // Creating player interface proxy
    Gio::DBus::Proxy::create_for_bus(
        Gio::DBus::BusType::SESSION, name, path, player_interface,
        [=, this](const Glib::RefPtr<Gio::AsyncResult> &result) {
          m_player_proxy = Gio::DBus::Proxy::create_for_bus_finish(result);

          if (!m_player_proxy) {
            std::cerr << "Error: Couldn't connect to media player via DBus"
                      << std::endl;
            return;
          }
          std::cout << "Successfully connected to media player" << std::endl;

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
            std::cerr
                << "Error: Couldn't subscribe to DBus PropertiesChanged signal"
                << e.what() << std::endl;
          }

          // Setting initial state
          update();
        });

    // Creating properties interface proxy
    m_properties_proxy = Gio::DBus::Proxy::create_for_bus_sync(
        Gio::DBus::BusType::SESSION, name, path, properties_interface);

  } catch (const Glib::Error &e) {
    std::cerr << "Error: Couldn't create DBus proxys" << e.what() << std::endl;
    // Set pointers to nullptr to indicate failure
    m_player_proxy.reset();
    m_properties_proxy.reset();
  }
}

void PlayerModule::on_prev_clicked() {
  if (m_player_proxy)
    m_player_proxy->call("Previous");
}

void PlayerModule::on_play_pause_clicked() {
  if (m_player_proxy)
    m_player_proxy->call("PlayPause");
}

void PlayerModule::on_next_clicked() {
  if (m_player_proxy)
    m_player_proxy->call("Next");
}

void PlayerModule::update() {
  // Set defaults if proxys weren't created
  if (!m_player_proxy || !m_properties_proxy) {
    m_status = "Unavailable";
    m_playing = false;
    m_paused = false;
    return;
  }

  // Update status and metadata
  get_status();
  get_metadata();
  get_progress();
  update_info();
}

void PlayerModule::get_status() {
  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring("org.mpris.MediaPlayer2.Player")),
         Glib::create_variant(Glib::ustring("PlaybackStatus"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters);

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
    std::cerr << "Error: Couldn't get status : " << e.what() << std::endl;
    m_status = "Error";
    m_playing = false;
    m_paused = false;
  }
}

void PlayerModule::get_metadata() {
  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring("org.mpris.MediaPlayer2.Player")),
         Glib::create_variant(Glib::ustring("Metadata"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters);

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
          m_track += " - ";

          for (int i = 0; i < artists.size(); i++) {
            m_track += artists[i];
            if (i < artists.size() - 1)
              m_track += ", ";
          }
        }
      }
    }

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
    std::cerr << "Error: Couldn't get metadata : " << e.what() << std::endl;
    m_track = "Nothing's playing currently...";
    m_duration = 0;
  }
}

Glib::ustring PlayerModule::format_time(gint64 microseconds) {
  if (microseconds == 0)
    return "0:00";

  gint64 seconds = microseconds / 1000000;
  gint64 minutes = seconds / 60;
  seconds %= 60;

  char buffer[6];
  std::snprintf(buffer, sizeof(buffer), "%ld:%02ld", minutes, seconds);
  return buffer;
}

void PlayerModule::get_progress() {
  if (!m_properties_proxy)
    return;

  try {
    auto parameters = Glib::VariantContainerBase::create_tuple(
        {Glib::create_variant(Glib::ustring("org.mpris.MediaPlayer2.Player")),
         Glib::create_variant(Glib::ustring("Position"))});

    auto call_result = m_properties_proxy->call_sync("Get", parameters);

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
    m_position = 0;
  }
}

bool PlayerModule::update_progress() {
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

void PlayerModule::update_info() {
  if (m_status == "Playing") {
    if (m_use_nerd_font)
      m_play_pause_button.set_label(m_icon_pause);
    else
      m_play_pause_button.set_icon_name(m_icon_pause);

    // Start the progress timer
    if (!m_progress_timeout.connected())
      m_progress_timeout = Glib::signal_timeout().connect(
          sigc::mem_fun(*this, &PlayerModule::update_progress), 1000);
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
