use std::{
    path::{Path, PathBuf},
    sync::OnceLock,
};

use anyhow::{Context, Result, anyhow};
use mlua::{FromLua, Lua, Table as LuaTable, Value as LuaValue};

use crate::{
    dynamic::SIGNAL_BUS,
    shell::gtk_bindings::{Anchors, Layer, Margins},
    widgets::Widget,
};

static CONFIG_PATH: OnceLock<PathBuf> = OnceLock::new();
pub static LUA: OnceLock<Lua> = OnceLock::new();

/// Helper function to get the directory of the currently loaded config file using `CONFIG_PATH`.
pub fn get_config_dir() -> Result<PathBuf> {
    match CONFIG_PATH.get() {
        Some(path) => Ok(path
            .parent()
            .context("Config path has no parent directory")?
            .to_path_buf()),
        None => anyhow::bail!("Couldn't get config path"),
    }
}

/// The global configuration for wayglance, loaded from a Lua script.
pub struct Config {
    /// The title of the window, used to build the `application_id` for GTK.
    pub title: String,
    /// An optional CSS style to apply to the window, specified as a path to a CSS file relative to
    /// the config file directory or an absolute path.
    pub style: Option<String>,
    /// The layer where the window will be placed.
    pub layer: Layer,
    /// Whether the window should reserve space on the screen (pushing other windows away) or
    /// overlap them. If true, will use  `auto_exclusive_zone_enable` from gtk layer shell.
    pub exclusive_zone: bool,
    /// Anchor points for the window to stick to specific edges of the monitor.
    pub anchors: Option<Anchors>,
    /// Margins to apply on each side of the window, specified in pixels.
    pub margins: Option<Margins>,
    /// A list of monitor connector names (e.g., "eDP-1", "HDMI-2"...). Empty means all monitors.
    pub monitors: Vec<String>,
    /// The child widget to display inside the window. The widget is defined as a Lua table with a
    /// `type` field (e.g., "button", "label"...) and other fields specific to that widget type.
    /// Widgets are built recursively so one may use container widgets to create complex layouts.
    pub child: Box<dyn Widget>,
}

impl Config {
    /// Loads the configuration from the specified Lua file.
    /// This also initializes the Lua environment and injects the `wayglance` global.
    /// The Lua file can return either a table (with the config data) or a function (which will be
    /// called to get the config).
    ///
    /// # Errors
    /// Returns an error if the file doesn't exist, if there's an error reading it, if the Lua code
    /// is invalid, or if the returned value is not a table or function.
    pub fn load(path: &str) -> Result<Self> {
        let path = Path::new(path);

        if !path.exists() {
            anyhow::bail!("Config file not found: {}", path.display());
        }

        CONFIG_PATH
            .set(path.to_path_buf())
            .map_err(|_| anyhow!("Couldn't set config path"))?;

        let content = std::fs::read_to_string(path)?;

        let lua = Lua::new();

        lua.load(include_str!("../../res/config.lua")).exec()?;
        lua.load(include_str!("../../res/widgets.lua")).exec()?;

        let globals = lua.globals();
        let wayglance: LuaTable = globals.get("wayglance")?;
        let emit_signal =
            lua.create_function(|_, (signal, data): (String, Option<LuaValue>)| {
                let data = data.unwrap_or(LuaValue::Nil);
                SIGNAL_BUS.with(|bus| bus.borrow().emit(&signal, data));
                Ok(())
            })?;
        wayglance.set("emitSignal", emit_signal)?;

        let value: LuaValue = lua.load(&content).set_name("data").eval()?;

        let config = match value {
            LuaValue::Function(f) => f.call::<Config>(())?,
            LuaValue::Table(_) => Config::from_lua(value, &lua)?,
            _ => {
                anyhow::bail!(
                    "Expected a function or table in config file, got {}",
                    value.type_name()
                );
            }
        };

        LUA.set(lua)
            .map_err(|_| anyhow!("Couldn't set Lua instance"))?;

        Ok(config)
    }
}

impl FromLua for Config {
    fn from_lua(value: LuaValue, _: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Config".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        let title = table.get("title")?;
        let style = table.get("style")?;
        let layer = table.get("layer")?;
        let exclusive_zone = table
            .get::<Option<bool>>("exclusive_zone")?
            .unwrap_or(false);
        let anchors = table.get("anchors")?;
        let margins = table.get("margins")?;
        let monitors = table
            .get::<Option<Vec<String>>>("monitors")?
            .unwrap_or_default();
        let child = table.get("child")?;

        Ok(Config {
            title,
            style,
            layer,
            exclusive_zone,
            anchors,
            margins,
            monitors,
            child,
        })
    }
}
