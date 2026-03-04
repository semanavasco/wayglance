use mlua::{Function as LuaFn, Lua, Value as LuaValue};

use crate::{dynamic::SIGNAL_BUS, lua::types::StringOrStrings};

/// Schedules the provided callback to be called repeatedly at the specified interval (in ms).
pub fn set_interval(lua: &Lua, (callback, interval): (LuaFn, u64)) -> mlua::Result<LuaValue> {
    let table = lua.create_table()?;
    table.set("__wayglance_dynamic", "interval")?;
    table.set("callback", callback)?;
    table.set("interval", interval)?;
    Ok(LuaValue::Table(table))
}

/// Listen for one or more signals and call the provided callback when they are emitted.
pub fn on_signal(
    lua: &Lua,
    (signals, callback): (StringOrStrings, mlua::Function),
) -> mlua::Result<LuaValue> {
    let table = lua.create_table()?;
    table.set("__wayglance_dynamic", "signal")?;
    table.set("signal", signals)?;
    table.set("callback", callback)?;
    Ok(LuaValue::Table(table))
}

/// Emit a signal with the given name and optional data payload.
pub fn emit_signal(_lua: &Lua, (signal, data): (String, Option<LuaValue>)) -> mlua::Result<()> {
    let data = data.unwrap_or(LuaValue::Nil);
    // Collect callbacks under a short borrow then call them after the borrow is
    // released to prevent re-entrancy panics
    let callbacks = SIGNAL_BUS.with(|bus| bus.borrow().callbacks_for(&signal));
    for cb in callbacks {
        cb(data.clone());
    }
    Ok(())
}
