use mlua::{Function as LuaFn, Lua, Table, Value as LuaValue};
use wayglance_macros::{LuaModule, lua_func};

use crate::{
    dynamic::{Interval, SIGNAL_BUS, Signal},
    lua::types::StringOrStrings,
    shell::Shell,
};

/// The `wayglance` module, which provides helper functions for dynamic bindings and event
/// handling.
#[allow(dead_code)]
#[derive(LuaModule)]
pub struct Wayglance;

/// Creates a new shell configuration.
#[lua_func(module = "wayglance")]
#[arg(name = "config", doc = "The global shell configuration.")]
#[ret(doc = "shell The shell object.")]
pub fn shell(config: Table) -> mlua::Result<Shell> {
    let title = config
        .get::<Option<String>>("title")?
        .unwrap_or_else(|| "wayglance".to_string());
    let style = config.get::<Option<String>>("style")?;

    Ok(Shell {
        title,
        style,
        windows: Vec::new(),
    })
}

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
#[ret(doc = "interval A table representing the interval timer.")]
pub fn set_interval(lua: &Lua, callback: LuaFn, interval: u64) -> mlua::Result<Interval> {
    Ok(Interval {
        callback: lua.create_registry_value(callback)?,
        interval,
    })
}

/// Listen for one or more signals and call the provided callback when they are emitted.
#[lua_func(name = "onSignal", skip = "lua", module = "wayglance")]
#[arg(name = "signals", doc = "The signal or signals to listen for.")]
#[arg(
    name = "callback",
    doc = "The callback to call when the signal(s) are emitted."
)]
#[ret(doc = "signal A table representing the signal listener.")]
pub fn on_signal(lua: &Lua, signals: StringOrStrings, callback: LuaFn) -> mlua::Result<Signal> {
    Ok(Signal {
        signals: match signals {
            StringOrStrings::Single(s) => vec![s],
            StringOrStrings::Multiple(v) => v,
        },
        callback: lua.create_registry_value(callback)?,
    })
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
