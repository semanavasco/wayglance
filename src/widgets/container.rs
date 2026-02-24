use anyhow::Result;
use gtk4::{Box as GtkBox, prelude::BoxExt};
use mlua::FromLua;

use crate::{
    shell::gtk_bindings::Orientation,
    widgets::{Properties, Widget},
};

pub struct Container {
    pub properties: Properties,
    pub orientation: Orientation,
    pub spacing: i32,
    pub children: Vec<Box<dyn Widget>>,
}

impl Widget for Container {
    fn build(&self) -> Result<gtk4::Widget> {
        let container = GtkBox::new(self.orientation.into(), self.spacing);

        self.properties.apply(&container);

        for child in &self.children {
            container.append(&child.build()?);
        }

        Ok(container.into())
    }
}

impl FromLua for Container {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let table = match &value {
            mlua::Value::Table(t) => t,
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
