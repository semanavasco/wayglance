//! This module handles the application shell, including GTK initialization, window management, and
//! monitor-specific configuration.

pub mod lifecycle;
mod style;
mod window;

use crate::shell::window::Window;
use anyhow::{Context, Result, anyhow};
use mlua::{IntoLua, Lua, Table as LuaTable, UserData, Value as LuaValue};
use std::{
    path::{Path, PathBuf},
    sync::OnceLock,
};
use wayglance_macros::{LuaClass, lua_func};

static PATH: OnceLock<PathBuf> = OnceLock::new();

/// Helper function to get the directory of the currently loaded config file.
pub fn get_config_dir() -> Result<PathBuf> {
    match PATH.get() {
        Some(path) => Ok(path
            .parent()
            .context("Config path has no parent directory")?
            .to_path_buf()),
        None => anyhow::bail!("Couldn't get config path"),
    }
}

/// The top-level configuration for the wayglance application shell.
///
/// A `Shell` acts as a container for global configuration (like the application title and styles)
/// and a list of [`Window`] definitions that define the UI structure.
#[derive(Clone, LuaClass)]
pub struct Shell {
    /// The global title of the shell, which determines the GTK `application_id`.
    pub title: String,
    /// Path to an optional global CSS stylesheet.
    pub style: Option<String>,
    /// All the window definitions registered in this shell.
    pub windows: Vec<Window>,
}

/// Adds a new window definition to the shell configuration.
#[lua_func(class = "Shell", skip = "this")]
#[arg(name = "name", doc = "The unique name of the window.")]
#[arg(
    name = "config",
    ty = "Window",
    doc = "The configuration for this window."
)]
fn window(this: &mut Shell, name: String, config: LuaTable) -> mlua::Result<()> {
    let template = Window::parse(name, config).map_err(mlua::Error::external)?;
    this.windows.push(template);
    Ok(())
}

impl UserData for Shell {
    fn add_methods<M: mlua::UserDataMethods<Self>>(methods: &mut M) {
        methods.add_method_mut(
            "window",
            |_lua, this, (name, config): (String, LuaTable)| window(this, name, config),
        );
    }
}

/// Detailed information about a physical monitor where a shell window is being displayed.
#[derive(Clone, LuaClass)]
pub struct Monitor {
    /// The name of the monitor (e.g., "eDP-1").
    pub name: String,
    /// The unique identifier for the monitor (currently defaults to -1).
    pub id: i128,
    /// The width of the monitor in pixels.
    pub width: i32,
    /// The height of the monitor in pixels.
    pub height: i32,
    /// The refresh rate of the monitor in Hz.
    pub refresh_rate: f32,
    /// The UI scale factor (e.g., 1.0, 2.0).
    pub scale: f32,
}

impl IntoLua for Monitor {
    fn into_lua(self, lua: &Lua) -> mlua::Result<LuaValue> {
        let table = lua.create_table()?;
        table.set("name", self.name)?;
        table.set("id", self.id)?;
        table.set("width", self.width)?;
        table.set("height", self.height)?;
        table.set("refresh_rate", self.refresh_rate)?;
        table.set("scale", self.scale)?;
        Ok(LuaValue::Table(table))
    }
}

impl From<&gtk4::gdk::Monitor> for Monitor {
    fn from(monitor: &gtk4::gdk::Monitor) -> Self {
        use gtk4::prelude::MonitorExt;
        let connector = monitor
            .connector()
            .map(|s: gtk4::glib::GString| s.to_string())
            .unwrap_or_else(|| "unknown".to_string());

        Monitor {
            name: connector,
            id: -1,
            width: monitor.geometry().width(),
            height: monitor.geometry().height(),
            refresh_rate: monitor.refresh_rate() as f32 / 1000.0,
            scale: monitor.scale_factor() as f32,
        }
    }
}

/// Loads the configuration from the specified Lua file.
pub fn load(path: &str) -> Result<Shell> {
    let path_ref = Path::new(path);
    if !path_ref.exists() {
        anyhow::bail!("Config file not found: {}", path_ref.display());
    }

    let path = path_ref
        .canonicalize()
        .context("Failed to canonicalize config path")?;

    PATH.set(path.clone())
        .map_err(|_| anyhow!("Couldn't set config path"))?;

    let content = std::fs::read_to_string(&path)?;

    let lua = Lua::new();

    // Add the config directory to the package search path
    if let Some(parent) = path.parent() {
        let parent_str = parent.to_string_lossy();
        let lua_code = format!(
            "package.path = package.path .. ';{}/?.lua;{}/?/init.lua'",
            parent_str, parent_str
        );
        lua.load(&lua_code).exec()?;
    }

    crate::lua::register_lua(&lua)?;

    let value: LuaValue = lua.load(&content).set_name("config").eval()?;

    let shell = match value {
        LuaValue::UserData(ud) => ud.borrow::<Shell>()?.clone(),
        _ => anyhow::bail!("Config script must return a wayglance.shell"),
    };

    crate::lua::LUA
        .set(lua)
        .map_err(|_| anyhow!("Couldn't set Lua instance"))?;

    Ok(shell)
}
