use std::any::type_name;

use mlua::FromLua;

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
    fn from_lua(value: mlua::Value, lua: &mlua::Lua) -> mlua::Result<Self> {
        if let mlua::Value::Table(ref t) = value
            && t.contains_key("__wayglance_gen").unwrap_or(false)
        {
            let callback: mlua::Function = t.get("callback")?;
            let registry = lua.create_registry_value(callback)?;
            let interval = t.get("interval")?;

            return Ok(MaybeDynamic::Interval {
                callback: registry,
                interval,
            });
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

