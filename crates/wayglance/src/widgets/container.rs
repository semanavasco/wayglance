use anyhow::Result;
use gtk4::{
    Box as GtkBox,
    prelude::{BoxExt, WidgetExt},
};
use mlua::{FromLua, Lua, Value as LuaValue};
use wayglance_macros::{LuaClass, WidgetBuilder};

use crate::{
    dynamic::{MaybeDynamic, bind_interval, bind_signals},
    lua::types::Orientation,
    widgets::{Properties, Widget},
};

/// A container widget that can hold multiple child widgets, arranged either horizontally or
/// vertically.
#[derive(LuaClass, WidgetBuilder)]
pub struct Container {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The orientation of the container.
    pub orientation: Orientation,
    /// The spacing between children in the container, in pixels.
    #[lua_attr(default = 0)]
    pub spacing: i32,
    /// The child widgets contained within this container.
    #[lua_attr(optional)]
    pub children: MaybeDynamic<Vec<Box<dyn Widget>>>,
}

impl Widget for Container {
    fn build(&self) -> Result<gtk4::Widget> {
        let container = GtkBox::new(self.orientation.into(), self.spacing);

        self.properties.apply(&container)?;

        match &self.children {
            MaybeDynamic::Static(children) => {
                for child in children {
                    container.append(&child.build()?);
                }
            }
            MaybeDynamic::Interval(signal) => {
                bind_interval(
                    &container,
                    &signal.callback,
                    signal.interval,
                    "children",
                    |w, children: Vec<Box<dyn Widget>>| {
                        while let Some(child) = w.first_child() {
                            w.remove(&child);
                        }
                        for child in children {
                            match child.build() {
                                Ok(child_widget) => w.append(&child_widget),
                                Err(e) => {
                                    tracing::error!("Failed to build child widget: {}", e);
                                }
                            }
                        }
                    },
                )?;
            }
            MaybeDynamic::Signal(signal) => {
                bind_signals(
                    &container,
                    &signal.callback,
                    &signal.signals,
                    "children",
                    |w, children: Vec<Box<dyn Widget>>| {
                        while let Some(child) = w.first_child() {
                            w.remove(&child);
                        }
                        for child in children {
                            match child.build() {
                                Ok(child_widget) => w.append(&child_widget),
                                Err(e) => {
                                    tracing::error!("Failed to build child widget: {}", e);
                                }
                            }
                        }
                    },
                )?;
            }
        }

        Ok(container.into())
    }
}

impl FromLua for Container {
    fn from_lua(value: LuaValue, _: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Container".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Container {
            properties: Properties::parse(table)?,
            orientation: table.get("orientation")?,
            spacing: table.get::<Option<i32>>("spacing")?.unwrap_or(0),
            children: table
                .get::<Option<MaybeDynamic<Vec<Box<dyn Widget>>>>>("children")?
                .unwrap_or(MaybeDynamic::Static(vec![])),
        })
    }
}
