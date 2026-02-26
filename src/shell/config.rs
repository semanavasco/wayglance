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

pub fn get_relative_config_dir() -> Result<PathBuf> {
    match CONFIG_PATH.get() {
        Some(path) => Ok(path
            .parent()
            .context("Config path has no parent directory")?
            .to_path_buf()),
        None => anyhow::bail!("Couldn't get config path"),
    }
}

pub struct Config {
    pub title: String,
    pub style: Option<String>,
    pub layer: Layer,
    pub exclusive_zone: bool,
    pub anchors: Option<Anchors>,
    pub margins: Option<Margins>,
    pub monitors: Vec<String>,
    pub child: Box<dyn Widget>,
}

impl Config {
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
