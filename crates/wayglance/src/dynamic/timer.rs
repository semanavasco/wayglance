use gtk4::glib;
use mlua::{Function as LuaFn, Lua, Table, Value as LuaValue};
use std::{
    sync::{Arc, Mutex},
    time::Duration,
};
use wayglance_macros::lua_func;

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
    ty = "CancelHandle",
    doc = "handle A handle that can be used to cancel the scheduled callback with :cancel()."
)]
pub fn set_interval(lua: &Lua, callback: LuaFn, interval: u64) -> mlua::Result<Table> {
    let callback_key = lua.create_registry_value(callback)?;

    let callback = lua.registry_value::<mlua::Function>(&callback_key)?;

    let source_id = glib::timeout_add_local(Duration::from_millis(interval), move || {
        if let Err(e) = callback.call::<LuaValue>(()) {
            tracing::error!("Error in setInterval callback: {}", e);
        }
        glib::ControlFlow::Continue
    });

    create_cancel_handle(lua, source_id)
}

/// Creates a cancel handle for a GLib timer source.
pub fn create_cancel_handle(lua: &Lua, source_id: gtk4::glib::SourceId) -> mlua::Result<Table> {
    let source_id = Arc::new(Mutex::new(Some(source_id)));
    let table = lua.create_table()?;

    let metatable = lua.create_table()?;
    let source_ref = source_id.clone();
    metatable.set(
        "cancel",
        lua.create_function(move |_, _this: Table| {
            if let Some(id) = source_ref.lock().unwrap().take() {
                id.remove();
            }
            Ok(())
        })?,
    )?;
    metatable.set("__index", metatable.clone())?;
    table.set_metatable(Some(metatable))?;

    Ok(table)
}
