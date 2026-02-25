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

impl FromLua for MaybeDynamic<String> {
    fn from_lua(value: mlua::Value, lua: &mlua::Lua) -> mlua::Result<Self> {
        let to = "MaybeDynamic<String>".to_string();
        let message = Some(
            "Expected a wayglance dynamic value (e.g. wayglance.setInterval(...))".to_string(),
        );

        match value {
            mlua::Value::String(s) => Ok(MaybeDynamic::Static(s.to_str()?.to_string())),
            mlua::Value::Table(t) => {
                if !t.contains_key("__wayglance_gen")? {
                    return Err(mlua::Error::FromLuaConversionError {
                        from: "table",
                        to,
                        message,
                    });
                }

                let callback: mlua::Function = t.get("callback")?;
                let registry = lua.create_registry_value(callback)?;
                let interval = t.get("interval")?;

                Ok(MaybeDynamic::Interval {
                    callback: registry,
                    interval,
                })
            }
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to,
                message,
            }),
        }
    }
}
