use anyhow::Result;
use gtk4::Label as GtkLabel;
use mlua::FromLua;

use crate::{
    dynamic::{MaybeDynamic, bind_interval},
    widgets::{Properties, Widget},
};

pub struct Label {
    pub properties: Properties,
    pub text: MaybeDynamic<String>,
}

impl Widget for Label {
    fn build(&self) -> Result<gtk4::Widget> {
        match &self.text {
            MaybeDynamic::Static(text) => {
                let label = GtkLabel::new(Some(text));
                self.properties.apply(&label)?;
                Ok(label.into())
            }
            MaybeDynamic::Interval { callback, interval } => {
                let label = GtkLabel::new(None);
                self.properties.apply(&label)?;

                let label_clone = label.clone();
                bind_interval(
                    &label_clone,
                    callback,
                    *interval,
                    "text",
                    |w, text: String| {
                        w.set_text(&text);
                    },
                )?;

                Ok(label.into())
            }
        }
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
