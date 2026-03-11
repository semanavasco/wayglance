use crate::{
    dynamic::MaybeReactive,
    widgets::{Properties, Widget},
};
use anyhow::Result;
use gtk4::Label as GtkLabel;
use mlua::{FromLua, Lua, Value as LuaValue};
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A simple widget that displays a text label.
#[derive(LuaClass, WidgetBuilder)]
pub struct Label {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The text content of the label. Can be a static string or a dynamic expression that
    /// evaluates to a string.
    pub text: MaybeReactive<String>,
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
