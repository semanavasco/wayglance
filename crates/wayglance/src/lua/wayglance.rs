use mlua::{Function as LuaFn, Lua, Value as LuaValue};
use wayglance_macros::{LuaModule, lua_func};

use crate::{dynamic::SIGNAL_BUS, lua::types::StringOrStrings};

/// The `wayglance` module, which provides helper functions for dynamic bindings and event
/// handling.
#[allow(dead_code)]
#[derive(LuaModule)]
pub struct Wayglance;

/// Schedules the provided callback to be called repeatedly at the specified interval (in ms).
#[lua_func(name = "setInterval", skip = "lua", module = "wayglance")]
#[arg(
    name = "callback",
    doc = "The callback to call after interval ms have passed."
)]
#[arg(
    name = "interval",
    doc = "The interval in milliseconds to wait before calling the callback."
)]
#[ret(
    doc = "interval A table representing the interval timer.",
    ty = "Interval"
)]
pub fn set_interval(lua: &Lua, callback: LuaFn, interval: u64) -> mlua::Result<LuaValue> {
    let table = lua.create_table()?;
    table.set("__wayglance_dynamic", "interval")?;
    table.set("callback", callback)?;
    table.set("interval", interval)?;
    Ok(LuaValue::Table(table))
}

/// Listen for one or more signals and call the provided callback when they are emitted.
#[lua_func(name = "onSignal", skip = "lua", module = "wayglance")]
#[arg(name = "signals", doc = "The signal or signals to listen for.")]
#[arg(
    name = "callback",
    doc = "The callback to call when the signal(s) are emitted."
)]
#[ret(
    doc = "signal A table representing the signal listener.",
    ty = "Signal"
)]
pub fn on_signal(lua: &Lua, signals: StringOrStrings, callback: LuaFn) -> mlua::Result<LuaValue> {
    let table = lua.create_table()?;
    table.set("__wayglance_dynamic", "signal")?;
    table.set("signal", signals)?;
    table.set("callback", callback)?;
    Ok(LuaValue::Table(table))
}

/// Emit a signal with the given name and optional data payload.
#[lua_func(name = "emitSignal", module = "wayglance")]
#[arg(name = "signal", doc = "The name of the signal to emit.")]
#[arg(
    name = "data",
    doc = "Optional data to include with the signal. Can be any Lua value."
)]
pub fn emit_signal(signal: String, data: Option<LuaValue>) -> mlua::Result<()> {
    let data = data.unwrap_or(LuaValue::Nil);
    // Collect callbacks under a short borrow then call them after the borrow is
    // released to prevent re-entrancy panics
    let callbacks = SIGNAL_BUS.with(|bus| bus.borrow().callbacks_for(&signal));
    for cb in callbacks {
        cb(data.clone());
    }
    Ok(())
}
