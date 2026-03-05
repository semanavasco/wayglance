use anyhow::Result;
use gtk4::{Box as GtkBox, prelude::BoxExt};
use mlua::{FromLua, Lua, Value as LuaValue};
use wayglance_macros::{LuaClass, WidgetBuilder};

use crate::{
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
    pub spacing: i32,
    /// The child widgets contained within this container.
    pub children: Vec<Box<dyn Widget>>,
}

impl Widget for Container {
    fn build(&self) -> Result<gtk4::Widget> {
        let container = GtkBox::new(self.orientation.into(), self.spacing);

        self.properties.apply(&container)?;

        for child in &self.children {
            container.append(&child.build()?);
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
                .get::<Option<Vec<Box<dyn Widget>>>>("children")?
                .unwrap_or_default(),
        })
    }
}
