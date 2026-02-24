use anyhow::Result;
use gtk4::Label as GtkLabel;
use mlua::FromLua;

use crate::widgets::{Properties, Widget};

pub struct Label {
    pub properties: Properties,
    pub text: String,
}

impl Widget for Label {
    fn build(&self) -> Result<gtk4::Widget> {
        let label = GtkLabel::new(Some(&self.text));

        self.properties.apply(&label);

        Ok(label.into())
    }
}

impl FromLua for Label {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let table = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Label".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Label {
            properties: Properties::parse(table)?,
            text: table.get("text")?,
        })
    }
}
