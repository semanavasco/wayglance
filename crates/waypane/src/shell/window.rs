use super::Monitor;
use crate::{
    lua::types::{Anchors, Layer, Margins},
    widgets::Widget,
};
use anyhow::{Context, Result};
use mlua::{FromLua, Table as LuaTable, Value as LuaValue};
use waypane_macros::LuaClass;

/// A window that can be instantiated on one or more monitors.
#[derive(Clone, LuaClass)]
pub struct Window {
    /// The unique name of this window, used for identification and debugging.
    #[lua_attr(default = "Gets the name from `window` method")]
    pub name: String,
    /// A list of monitor connector names to display this window on. If empty, all monitors.
    #[lua_attr(optional)]
    pub monitors: Vec<String>,
    /// The layer to display this window on.
    pub layer: Layer,
    /// Whether this window should reserve space on the monitor (like a panel) or be free-floating.
    #[lua_attr(default = false)]
    pub exclusive_zone: bool,
    /// The edges of the monitor to anchor this window to.
    #[lua_attr(optional)]
    pub anchors: Anchors,
    /// The margins to apply on each edge when anchored.
    #[lua_attr(optional)]
    pub margins: Margins,
    /// The layout of the window. Can be a table (static widget) or a function (monitor) -> widget.
    pub layout: LuaValue,
}

impl Window {
    /// Parses a window definition from a Lua table.
    pub fn parse(name: String, table: LuaTable) -> Result<Self> {
        Ok(Window {
            name,
            monitors: table
                .get::<Option<Vec<String>>>("monitors")?
                .unwrap_or_default(),
            layer: table.get("layer")?,
            exclusive_zone: table
                .get::<Option<bool>>("exclusive_zone")?
                .unwrap_or(false),
            anchors: table.get::<Option<Anchors>>("anchors")?.unwrap_or_default(),
            margins: table.get::<Option<Margins>>("margins")?.unwrap_or_default(),
            layout: table.get("layout")?,
        })
    }

    /// Instantiates the window for a specific monitor.
    ///
    /// If the `layout` is a function, it is called with the monitor information to produce
    /// the widget tree. Otherwise, it is treated as a static widget table.
    pub fn instantiate(&self, monitor: &Monitor) -> Result<WindowInstance> {
        let lua = crate::lua::LUA.get().context("Lua not initialized")?;

        let widget_value = match &self.layout {
            LuaValue::Function(f) => f.call::<LuaValue>(monitor.clone())?,
            LuaValue::Table(_) => self.layout.clone(),
            _ => anyhow::bail!("Window 'layout' must be a widget table or a function"),
        };

        let child = Box::<dyn Widget>::from_lua(widget_value, lua)?;

        Ok(WindowInstance {
            name: self.name.clone(),
            layer: self.layer,
            exclusive_zone: self.exclusive_zone,
            anchors: self.anchors.clone(),
            margins: self.margins.clone(),
            child,
        })
    }
}

/// A specific instance of a [`Window`] bound to a physical monitor.
pub struct WindowInstance {
    /// The name of the window instance.
    pub name: String,
    /// The layer where this window is placed.
    pub layer: Layer,
    /// Whether the window has an exclusive zone (reserves space).
    pub exclusive_zone: bool,
    /// The anchor points for this window.
    pub anchors: Anchors,
    /// The margins around the window.
    pub margins: Margins,
    /// The root widget of the window.
    pub child: Box<dyn Widget>,
}
