use anyhow::Result;
use gtk4::Label as GtkLabel;
use mlua::{FromLua, Lua, Value as LuaValue};

use crate::{
    dynamic::MaybeDynamic,
    widgets::{Properties, Widget},
};

pub struct Label {
    pub properties: Properties,
    pub text: MaybeDynamic<String>,
}

impl Widget for Label {
    fn build(&self) -> Result<gtk4::Widget> {
        let label = GtkLabel::new(None);
        self.properties.apply(&label)?;

        self.text.bind(&label, "text", |w, text| {
            w.set_text(&text);
        })?;

        Ok(label.into())
    }
}

impl FromLua for Label {
    fn from_lua(value: LuaValue, _: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
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
