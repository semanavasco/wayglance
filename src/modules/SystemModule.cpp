#include "modules/SystemModule.hpp"
#include <fstream>
#include <sstream>
#include <string>

// Constructor
SystemModule::SystemModule(const nlohmann::json &config) : BaseModule(config) {
  set_halign(Gtk::Align::CENTER);
  set_valign(Gtk::Align::CENTER);
  set_name("module-system");

  // Read config
  load_config(config);

  // Setup widgets
  if (m_cpu_active) {
    m_cpu_label.set_text("CPU: ...");
    m_cpu_label.set_name("system-cpu-label");
    m_cpu_label.add_css_class("system-labels");
    append(m_cpu_label);
  }

  if (m_ram_active) {
    m_ram_label.set_text("RAM: ...");
    m_ram_label.set_name("system-ram-label");
    m_ram_label.add_css_class("system-labels");
    append(m_ram_label);
  }

  if (m_net_active) {
    m_net_label.set_text("NET: ...");
    m_net_label.set_name("system-net-label");
    m_net_label.add_css_class("system-labels");
    append(m_net_label);
  }

  // Start the timer
  on_update_timer();
  Glib::signal_timeout().connect(
      sigc::mem_fun(*this, &SystemModule::on_update_timer), m_update_interval);
}

// Destructor
SystemModule::~SystemModule() {}

// Methods
void SystemModule::load_config(const nlohmann::json &config) {
  m_update_interval = config.value("update-interval", 1000);

  // CPU config
  const auto &cpu_config = config.value("cpu", nlohmann::json::object());
  m_cpu_active = cpu_config.value("active", true);
  m_cpu_format = cpu_config.value("format", "CPU: {usage}%");

  // RAM config
  const auto &ram_config = config.value("ram", nlohmann::json::object());
  m_ram_active = ram_config.value("active", true);
  m_ram_format = ram_config.value("format", "RAM: {usage}%");

  // Network config
  const auto &net_config = config.value("net", nlohmann::json::object());
  m_net_active = net_config.value("active", true);
  m_net_format = net_config.value("format", "NET: {download} / {upload}");
  m_net_interface = net_config.value("interface", "wlan0");
}

bool SystemModule::on_update_timer() {
  if (m_cpu_active)
    update_cpu_usage();
  if (m_ram_active)
    update_ram_usage();
  if (m_net_active)
    update_net_usage();

  return true;
}

void SystemModule::update_cpu_usage() {
  std::ifstream stat_file("/proc/stat");
  std::string line;
  long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

  // Getting the "cpu" line
  std::getline(stat_file, line);
  stat_file.close();

  // Using a string stream to easily parse the numbers
  std::stringstream ss(line);
  std::string cpu_str;
  ss >> cpu_str >> user >> nice >> system >> idle >> iowait >> irq >> softirq >>
      steal >> guest >> guest_nice;

  // Calculate total times
  long current_idle_time = idle + iowait;
  long current_total_time =
      user + nice + system + current_idle_time + irq + softirq + steal;

  // If it's the first time, store the values
  if (m_prev_total_time == 0) {
    m_prev_total_time = current_total_time;
    m_prev_idle_time = current_idle_time;
    return;
  }

  // Else, calculate the difference from previous measurement
  long total_diff = current_total_time - m_prev_total_time;
  long idle_diff = current_idle_time - m_prev_idle_time;

  // CPU usage percentage
  double cpu_usage = 0.0;
  if (total_diff > 0)
    cpu_usage = (1.0 - static_cast<double>(idle_diff) / total_diff) * 100.0;

  // Updating the label
  m_cpu_label.set_text(format_label(m_cpu_format, "{usage}", cpu_usage));

  // Update previous values
  m_prev_total_time = current_total_time;
  m_prev_idle_time = current_idle_time;
}

void SystemModule::update_ram_usage() {
  long mem_total(0), mem_available(0);

  std::ifstream stat_file("/proc/meminfo");
  if (!stat_file.is_open())
    return;

  std::string line;
  while (std::getline(stat_file, line)) {
    std::stringstream ss(line);
    std::string key;
    long value;

    ss >> key >> value;

    if (key == "MemTotal:")
      mem_total = value;
    else if (key == "MemAvailable:")
      mem_available = value;
    else
      continue;

    if (mem_total > 0 && mem_available > 0)
      break;
  }
  stat_file.close();

  // RAM usage percentage
  if (mem_total > 0 && mem_available > 0) {
    long mem_used = mem_total - mem_available;
    double ram_usage = (static_cast<double>(mem_used) / mem_total) * 100;

    // Updating label
    m_ram_label.set_text(format_label(m_ram_format, "{usage}", ram_usage));
  } else
    m_ram_label.set_text("RAM: N/A");
}

void SystemModule::update_net_usage() {
  long long new_bytes_received = 0;
  long long new_bytes_sent = 0;

  std::ifstream stat_file("/proc/net/dev");
  if (!stat_file.is_open())
    return;

  std::string line;
  bool found_interface = false;
  while (std::getline(stat_file, line)) {
    if (line.find(m_net_interface + ":") != std::string::npos) {
      std::string interface_str;
      long packets_r, errs_r, drop_r, fifo_r, frame_r, compressed_r,
          multicast_r, packets_s, errs_s, drop_s, fifo_s, colls_s, carrier_s,
          compressed_s;

      std::stringstream ss(line);
      ss >> interface_str >> new_bytes_received >> packets_r >> errs_r >>
          drop_r >> fifo_r >> frame_r >> compressed_r >> multicast_r >>
          new_bytes_sent >> packets_s >> errs_s >> drop_s >> fifo_s >>
          colls_s >> carrier_s >> compressed_s;

      found_interface = true;
      break;
    }
  }
  stat_file.close();

  if (!found_interface) {
    m_net_label.set_text("NET: " + m_net_interface + "?");
    return;
  }

  if (m_prev_bytes_received > 0 && m_prev_bytes_sent > 0) {
    double interval_s = m_update_interval / 1000.0;
    double download_spd_bps =
        static_cast<double>(new_bytes_received - m_prev_bytes_received) /
        interval_s;
    double upload_spd_bps =
        static_cast<double>(new_bytes_sent - m_prev_bytes_sent) / interval_s;

    // Updating label
    m_net_label.set_text(format_net_label(download_spd_bps, upload_spd_bps));
  }

  // Updating previous values
  m_prev_bytes_received = new_bytes_received;
  m_prev_bytes_sent = new_bytes_sent;
}

Glib::ustring SystemModule::format_label(const std::string &format,
                                         const std::string &key, double value) {
  std::string formatted_text = format;
  std::string value_str =
      Glib::ustring::format(std::fixed, std::setprecision(1), value);

  size_t pos = formatted_text.find(key);
  if (pos != std::string::npos)
    formatted_text.replace(pos, key.length(), value_str);

  return formatted_text;
}

Glib::ustring SystemModule::format_net_label(double download, double upload) {
  std::string net_text = m_net_format;

  size_t pos = net_text.find("{download}");
  if (pos != std::string::npos)
    net_text.replace(pos, std::string("{download}").length(),
                     format_speed(download));

  pos = net_text.find("{upload}");
  if (pos != std::string::npos)
    net_text.replace(pos, std::string("{upload}").length(),
                     format_speed(upload));

  return net_text;
}

Glib::ustring SystemModule::format_speed(double speed_bps) {
  if (speed_bps < 1024)
    return Glib::ustring::format(std::fixed, std::setprecision(0), speed_bps) +
           " B/s";
  else if (speed_bps < 1024 * 1024)
    return Glib::ustring::format(std::fixed, std::setprecision(1),
                                 speed_bps / 1024) +
           " KB/s";
  else
    return Glib::ustring::format(std::fixed, std::setprecision(1),
                                 speed_bps / (1024 * 1024)) +
           " MB/s";
}
