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
    let globals = lua.globals();

    // Register widget builder functions as Lua globals
    // Each builder takes a config table, sets its `type` field, and returns it
    for factory in inventory::iter::<StubFactory> {
        if let Stub::WidgetBuilder(wb) = (factory.build)() {
            let type_name = wb.type_name;
            globals.set(
                wb.name,
                lua.create_function(move |_, config: mlua::Table| {
                    config.set("type", type_name)?;
                    Ok(config)
                })?,
            )?;
        }
    }

    let wayglance = lua.create_table()?;
    globals.set("wayglance", &wayglance)?;

    wayglance.set(
        "setInterval",
        lua.create_function(|lua, (cb, ms)| wayglance::set_interval(lua, cb, ms))?,
    )?;

    wayglance.set(
        "onSignal",
        lua.create_function(|lua, (sigs, cb)| wayglance::on_signal(lua, sigs, cb))?,
    )?;

    wayglance.set(
        "emitSignal",
        lua.create_function(|_, (sig, data)| wayglance::emit_signal(sig, data))?,
    )?;

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
    let mut functions = Vec::new();
    let mut builders = Vec::new();

    for factory in inventory::iter::<StubFactory> {
        match (factory.build)() {
            Stub::Enum(e) => enums.push(e.to_string()),
            Stub::Class(c) => classes.push(c.to_string()),
            Stub::Function(f) => functions.push(f.to_string()),
            Stub::WidgetBuilder(wb) => builders.push(wb.to_string()),
        }
    }

    let mut out = String::new();
    out.push_str("---@meta\n\n");

    if !enums.is_empty() {
        out.push_str(&enums.join("\n\n"));
    }

    if !classes.is_empty() {
        out.push_str("\n\n");
        out.push_str(&classes.join("\n"));
    }

    if !functions.is_empty() {
        out.push('\n');
        out.push_str(&functions.join("\n\n"));
    }

    if !builders.is_empty() {
        out.push_str("\n\n");
        out.push_str(&builders.join("\n\n"));
    }

    out
}
