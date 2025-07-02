# Wayglance

<div align="center">

![Wayglance](https://img.shields.io/badge/version-0.0.33-red)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue?logo=cplusplus)
![CMake](https://img.shields.io/badge/CMake-3.15+-blue?logo=cmake)
![GTK](https://img.shields.io/badge/GTK-4.0-blue?logo=gtk)

_A desktop overlay widget for Wayland that brings system information and media controls directly to your wallpaper!_

</div>

## ⚠️ Disclaimer

**This project was created as a learning exercise and is my first-ever C++ project!**

While I've put significant effort into following modern C++ best practices and creating a well-structured codebase, please be aware that:

- 🐛 **Bugs are expected** - There **WILL** be memory leaks, crashes, or unexpected behavior (and I will fix them eventually)
- 🔧 **Not production-ready** - This is **NOT** recommended for everyday use
- 📚 **Learning in progress** - I'm still learning C++ and **HAVE** made rookie mistakes
- 🚧 **Work in Progress** - The project will continue to evolve as I learn more, eventually reaching a usable state

**My goal is to keep improving this project until it becomes truly usable!** Every mistake is a learning opportunity, and I'm committed to making this better over time.

## 🤝 Contributing

**Contributions are not currently accepted** because I want to keep learning by implementing features myself. However:

- 📂 **Feel free to fork** this project and experiment with it
- 🐞 **Bug reports are welcome** - they help me learn what needs fixing
- 💬 **Feedback is more than welcome** - constructive criticism helps me grow
- 🎓 **Want to teach me?** I'd absolutely love to learn from experienced developers!

If you're interested in helping me learn C++, Wayland development, or modern software engineering practices, please reach out! I'm eager to learn from the community.

## 📖 What is Wayglance?

Wayglance is a desktop overlay application for Wayland compositors that displays customizable widgets directly on your desktop background. It creates a transparent layer-shell window that shows:

### 🕒 Date & Time Module

- Large, customizable clock display
- Configurable date formats
- Real-time updates

### 🎵 Media Player Module

- Current track information
- Playback controls (play/pause/previous/next)
- Progress bar with time display
- MPRIS integration (works with Spotify, VLC, etc.)
- Support for Nerd Fonts icons

### 📊 System Monitor Module

- **CPU Usage** - Real-time processor utilization
- **RAM Usage** - Memory consumption monitoring
- **Network Stats** - Download/upload speeds per interface

All modules are **highly configurable** through JSON configuration files and **fully stylable** with CSS. Many modules are to come.

## ✨ Features

- 🖥️ **Multi-monitor support** - Automatically creates instances for each display
- 🎨 **Fully customizable** - JSON configuration + CSS styling
- 🔧 **Modular design** - Enable/disable individual components
- 🪟 **Wayland native** - Uses gtk4-layer-shell for proper integration
- 📦 **Easy configuration** - Built-in defaults with `--create-defaults`

## 🚀 Quick Start

### Prerequisites

**System Requirements:**

- Linux with Wayland compositor
- GTK4 development libraries
- gtk4-layer-shell
- CMake 3.15+
- vcpkg (for dependencies)
- C++20 compatible compiler

**Install dependencies:**

```bash
# Arch Linux
sudo pacman -S base-devel gtkmm-4.0 gtk4-layer-shell cmake vcpkg

# VCPKG dependencies
vcpkg install nlohmann-json bfgroup-lyra
```

### Building

```bash
# Clone the repository
git clone https://github.com/semanavasco/wayglance.git
cd wayglance

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=/path/to/your/vcpkg.cmake

# Build
cmake --build .
```

### Initial Setup

Create default configuration files:

```bash
./wayglance --create-defaults
```

This creates:

- `~/.config/wayglance/config.json` - Main configuration
- `~/.config/wayglance/style.css` - Stylesheet

### Running

```bash
# Run with default configuration at ~/.config/wayglance/* or defaults
./wayglance

# Run with custom config
./wayglance --config /path/to/config.json --style /path/to/style.css

# Show help
./wayglance --help
```

## ⚙️ Configuration

### Module Configuration

The main configuration is in `~/.config/wayglance/config.json`:

```json
{
  "modules": [
    { "name": "date", "position": "middle-center" },
    { "name": "player", "position": "middle-center" },
    { "name": "system", "position": "bottom-center" }
  ],
  "date": {
    "time_format": "%H:%M",
    "date_format": "%A, %d %B %Y"
  },
  "player": {
    "player": "spotify",
    "nerd-font": false
  },
  "system": {
    "update-interval": 1000,
    "cpu": { "active": true, "format": "CPU: {usage}%" },
    "ram": { "active": true, "format": "RAM: {usage}%" },
    "net": {
      "active": true,
      "format": "NET: {download} / {upload}",
      "interface": "wlan0"
    }
  }
}
```

### Position Options

- `top-left`, `top-center`, `top-right`
- `middle-left`, `middle-center`, `middle-right`
- `bottom-left`, `bottom-center`, `bottom-right`

### Styling

Customize appearance in `~/.config/wayglance/style.css`:

```css
#date-time-label {
  font-size: 120pt;
  font-weight: bold;
  color: #cba6f7;
}

#module-player {
  margin-top: 50pt;
}

.player-buttons:hover {
  background-color: rgba(255, 255, 255, 0.1);
}
```

## 🏗️ Architecture

The project tries to follow modern C++ design patterns:

```
wayglance/
├── include/               # Header files
│   ├── managers/          # Configuration and client management
│   ├── modules/           # Widget modules (date, player, system)
│   ├── module.hpp         # Base module class
│   └── shell.hpp          # Main window shell
├── src/                   # Implementation files
├── CMakeLists.txt         # Build configuration
└── README.md
```

### Key Components

- **`wayglance::Shell`** - Main application window with layer-shell integration
- **`wayglance::Module`** - Base class for all widgets
- **`wayglance::managers::Config`** - Configuration loading and management
- **`wayglance::managers::Client`** - Application lifecycle and multi-monitor handling

## 🐛 Known Issues

As a learning project, there are several areas I'm aware need improvement:

- Memory management could be more robust
- Error handling needs expansion
- Configuration validation is minimal
- No unit tests yet
- Performance optimizations needed
- And more...

## 🎯 Roadmap

**Short-term goals:**

- [🔄] Add proper exception handling and validation
- [🔄] Improve managers
- [⏳] Implement configuration validation
- [⏳] Add logging system
- [⏳] Create unit tests
- [⏳] Performance optimizations

**Long-term goals:**

- [⏳] Hot-reload configuration
- [⏳] Pomodoro module
- [⏳] Weather module with API integration
- [⏳] Workspace/window information module
- [⏳] System notifications integration
- [⏳] Battery status module for laptops
- [⏳] Bluetooth device status module
- [⏳] Plugin system for custom modules
- [⏳] Package for major distributions

## 🙏 Special Thanks To

- The GTK and GTKmm teams for excellent documentation
- The Wayland community for layer-shell protocols
- [Alexays](https://github.com/Alexays) for making waybar (which got me interested in making this project)
- Everyone who provides feedback and helps me learn!

---

<div align="center">

**Remember: This is a learning project!** 📚

Feedback, suggestions, and teaching opportunities are highly appreciated!

[Report Bug](https://github.com/semanavasco/wayglance/labels/bug) · [Request Feature](https://github.com/semanavasco/wayglance/labels/enhancement) · [Ask Question](https://github.com/semanavasco/wayglance/labels/question)

</div>
