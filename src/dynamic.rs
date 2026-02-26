use std::cell::Cell;
use std::{any::type_name, time::Duration};

use anyhow::{Context, Result};
use gtk4::{
    glib::{self, object::IsA},
    prelude::WidgetExt,
};
use mlua::{FromLua, Lua, Value as LuaValue};

use crate::shell::config::LUA;

/// A value that is either resolved statically at parse time, or computed dynamically from a lua
/// callback.
pub enum MaybeDynamic<T> {
    /// A plain value of type `T`.
    Static(T),
    /// A dynamic value of type `T` returned from the callback every `interval` ms.
    Interval {
        callback: mlua::RegistryKey,
        interval: u64,
    },
    // TODO Add support for dynamic values updated from a signal instead of an interval
    // For example: after a button click
}

impl<T> FromLua for MaybeDynamic<T>
where
    T: FromLua,
{
    fn from_lua(value: LuaValue, lua: &Lua) -> mlua::Result<Self> {
        if let LuaValue::Table(ref t) = value
            && let Ok(wayglance_d) = t.get::<String>("__wayglance_dynamic")
        {
            match wayglance_d.as_str() {
                "interval" => {
                    let callback: mlua::Function = t.get("callback")?;
                    let registry = lua.create_registry_value(callback)?;
                    let interval = t.get("interval")?;

                    return Ok(MaybeDynamic::Interval {
                        callback: registry,
                        interval,
                    });
                }
                _ => {
                    return Err(mlua::Error::FromLuaConversionError {
                        from: "string",
                        to: type_name::<MaybeDynamic<T>>().to_string(),
                        message: Some(
                            "Invalid dynamic value type (expected 'interval')".to_string(),
                        ),
                    });
                }
            }
        }

        let val_type = value.type_name();
        T::from_lua(value, lua)
            .map(MaybeDynamic::Static)
            .map_err(|_| mlua::Error::FromLuaConversionError {
                from: val_type,
                to: type_name::<MaybeDynamic<T>>().to_string(),
                message: Some(
                    "Expected a wayglance dynamic value (e.g. wayglance.setInterval(...))"
                        .to_string(),
                ),
            })
    }
}

impl<T> MaybeDynamic<T>
where
    T: FromLua + Clone,
{
    pub fn bind<W, F>(&self, widget: &W, prop_name: &'static str, mut apply_fn: F) -> Result<()>
    where
        W: IsA<gtk4::Widget>,
        F: FnMut(&W, T) + 'static,
    {
        match self {
            MaybeDynamic::Static(val) => {
                apply_fn(widget, val.clone());
                Ok(())
            }
            MaybeDynamic::Interval { callback, interval } => {
                bind_interval(widget, callback, *interval, prop_name, apply_fn)
            }
        }
    }
}

fn bind_interval<T, W, F>(
    widget: &W,
    callback_key: &mlua::RegistryKey,
    interval: u64,
    prop_name: &'static str,
    mut apply_fn: F,
) -> Result<()>
where
    T: FromLua,
    W: IsA<gtk4::Widget>,
    F: FnMut(&W, T) + 'static,
{
    let lua = LUA.get().context("Lua instance not initialized")?;
    let callback = lua.registry_value::<mlua::Function>(callback_key)?;
    let widget_clone = widget.clone();

    callback
        .call::<T>(())
        .map(|val| apply_fn(&widget_clone, val))?;

    let source_id = glib::timeout_add_local(Duration::from_millis(interval), move || {
        match callback.call::<T>(()) {
            Ok(val) => apply_fn(&widget_clone, val),
            Err(e) => tracing::error!("Error calling Lua callback for {}: {}", prop_name, e),
        }
        glib::ControlFlow::Continue
    });

    let source_id = Cell::new(Some(source_id));
    widget.connect_destroy(move |_| {
        if let Some(id) = source_id.take() {
            id.remove();
        }
    });
    Ok(())
}
