# Getting Started

This guide will help you set up and run your first wayglance configuration.

For now, wayglance can only be installed from source.

## Prerequisites

Before building wayglance, ensure you have the following installed:

- **Rust**: The Rust toolchain (use [rustup](https://rustup.rs/) to install).
- **GTK4**: The development libraries for GTK4.
- **gtk4-layer-shell**: The development libraries for gtk4-layer-shell.
- **Lua**: The Lua interpreter and development libraries (`lua5.4`).
- **Wayland**: A Wayland compositor (Hyprland is recommended for full feature support).

> **Note:** The exact package names may vary depending on your Linux distribution. Please refer to your distribution's package manager for the correct names.

### Building

Start by cloning the repository and navigating into it:

```bash
git clone https://github.com/semanavasco/wayglance.git
```

```bash
cd wayglance
```

Then, build the project using Cargo:

```bash
cargo build --release --no-default-features
```

Or with the **-\-features** flag to enable additional features (currently only `hyprland` available):

```bash
cargo build --release --no-default-features --features hyprland
```

### Installation

You can also install wayglance system-wide using Cargo:

```bash
cargo install --path crates/wayglance --no-default-features --features hyprland
```

## Running your First Widget

wayglance requires a Lua configuration file to define its UI and behavior. You can run it by passing the path to your config:

```bash
wayglance run examples/bar.lua
```

You can also set the log level using the `-l` or `--log-level` flag (defaults to `info`):

```bash
wayglance run examples/bar.lua --log-level debug
```

Available log levels are: `error`, `warn`, `info`, `debug`, and `trace`. Pass nonsense value to disable logging entirely (e.g. `none`).

## IDE Integration (Recommended)

To get autocompletion and type hints in your Lua editor, generate a stubs file:

```bash
wayglance gen-stubs > stubs.lua
```

Place this file in your project directory. If you're using the [Lua Language Server](https://luals.github.io/), it will automatically pick up the definitions.

> **Note:** You may need an additional `.luarc.json` file to supress warnings about undefined globals. Here's an example:
>
> ```json
> {
>   "diagnostics": {
>     "globals": ["Label", "Container", "Button", ..., "wayglance"]
>   }
> }
> ```

## Basic Config Structure

A minimal wayglance configuration file (`config.lua`) looks like this:

```lua
local shell = wayglance.shell({
  title = "My Bar",
})

shell:window("main-window", {
  layer = "top",
  anchors = { top = true, left = true, right = true },

  layout = Label({
    text = "Hello, wayglance!",
  })
})

return shell
```

Save this to a file and run it with wayglance to see your first widget in action!
