//! Lua runtime bootstrap and shared Lua state.
//!
//! This module is responsible for:
//! - setting up the global `wayglance` Lua table with built-in helpers and runtime bindings
//! - storing the process-wide [`LUA`] instance used by dynamic bindings and event forwarding
//! - generating Lua stubs for all Rust-defined Lua classes and functions

pub mod stubs;
pub mod types;
mod wayglance;

use std::sync::OnceLock;

use anyhow::Result;
use mlua::Lua;

use crate::lua::stubs::{Stub, StubFactory};

/// Global Lua instance used by dynamic bindings and modules event forwarding.
/// This is set during config loading, after the Lua environment is initialized and the config file
/// is loaded. It is expected to be initialized by the time any dynamic bindings or events are
/// used, since they can only be used in the config file which is loaded after initialization.
pub static LUA: OnceLock<Lua> = OnceLock::new();

/// Initializes the Lua environment for a new shell instance.
///
/// Loads built-in helpers (`setInterval`, `onSignal`, widget constructors) and registers all
/// runtime bindings under the global `wayglance` table, including `emitSignal` and any
/// window-manager-specific subtable (e.g. `wayglance.hyprland`).
///
/// Must be called before the user config is evaluated and before [`LUA`] is set.
pub fn register_lua(lua: &Lua) -> Result<()> {
    lua.load(include_str!("../../../../res/widgets.lua"))
        .exec()?;

    let globals = lua.globals();
    let wayglance = lua.create_table()?;
    globals.set("wayglance", &wayglance)?;

    wayglance.set("setInterval", lua.create_function(wayglance::set_interval)?)?;
    wayglance.set("onSignal", lua.create_function(wayglance::on_signal)?)?;
    wayglance.set("emitSignal", lua.create_function(wayglance::emit_signal)?)?;

    // Inject Lua bindings for the window manager, if any are enabled
    // They are injected under a `wayglance.<wm_name>` table, e.g. `wayglance.hyprland`
    #[cfg(any(feature = "hyprland"))]
    crate::modules::wm::register_lua(lua, &wayglance)?;

    Ok(())
}

/// Generates Lua stubs for all Lua classes and functions defined in Rust, to provide better
/// autocompletion and type hints in the user config when using an LSP that supports it.
pub fn gen_stubs() -> String {
    let mut enums = Vec::new();
    let mut classes = Vec::new();

    for factory in inventory::iter::<StubFactory> {
        match (factory.build)() {
            Stub::Enum(e) => enums.push(e.to_string()),
            Stub::Class(c) => classes.push(c.to_string()),
        }
    }

    let mut out = String::new();
    out.push_str("---@meta\n\n");

    out.push_str(&enums.join("\n\n"));
    out.push_str("\n\n");
    out.push_str(&classes.join("\n"));
    out
}
