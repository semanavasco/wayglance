# Modules

`wayglance` can be extended with modules that provide integration with external tools and services. These modules typically expose a set of Lua functions and signals.

## Built-in Modules

- **[Hyprland](./hyprland.md)**: Deep integration with the Hyprland window manager, allowing you to react to workspace changes, window focus, and more.

## Using Modules

Modules are available as sub-tables of the global `wayglance` table.

```lua
-- Using the Hyprland module
local workspaces = wayglance.hyprland.getWorkspaces()
```

## Community Modules

In the future, `wayglance` may support a plugin system or a community-driven set of modules. If you're interested in developing your own module, check out the source code of the Hyprland module as a reference.
