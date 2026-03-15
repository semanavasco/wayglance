use crate::shell::Shell;
use mlua::Table;
use waypane_macros::{LuaModule, lua_func};

/// The `waypane` module, which provides helper functions for dynamic bindings and event
/// handling.
#[allow(dead_code)]
#[derive(LuaModule)]
struct Waypane;

/// Creates a new shell configuration.
#[lua_func(module = "waypane")]
#[arg(name = "config", doc = "The global shell configuration.")]
#[ret(doc = "shell The shell object.")]
pub fn shell(config: Table) -> mlua::Result<Shell> {
    let title = config
        .get::<Option<String>>("title")?
        .unwrap_or_else(|| "waypane".to_string());
    let style = config.get::<Option<String>>("style")?;

    Ok(Shell {
        title,
        style,
        windows: Vec::new(),
    })
}
