mod button;
mod container;
mod label;

use anyhow::Result;
use gtk4::{glib::object::IsA, prelude::WidgetExt};
use mlua::FromLua;

use crate::shell::gtk_bindings::Alignment;

pub trait Widget {
    fn build(&self) -> Result<gtk4::Widget>;
}

impl FromLua for Box<dyn Widget> {
    fn from_lua(value: mlua::Value, lua: &mlua::Lua) -> mlua::Result<Self> {
        let table = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Widget".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        let widget_type: String = table.get("type")?;

        match widget_type.as_str() {
            "button" => {
                let button = button::Button::from_lua(value, lua)?;
                Ok(Box::new(button))
            }
            "container" => {
                let container = container::Container::from_lua(value, lua)?;
                Ok(Box::new(container))
            }
            "label" => {
                let label = label::Label::from_lua(value, lua)?;
                Ok(Box::new(label))
            }
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to: "Widget".to_string(),
                message: Some(format!("Unknown widget type: {}", widget_type)),
            }),
        }
    }
}

struct Properties {
    pub id: Option<String>,
    pub class_list: Vec<String>,
    pub halign: Option<Alignment>,
    pub valign: Option<Alignment>,
    pub hexpand: bool,
    pub vexpand: bool,
}

impl Properties {
    fn parse(table: &mlua::Table) -> mlua::Result<Self> {
        Ok(Properties {
            id: table.get("id")?,
            class_list: table
                .get::<Option<Vec<String>>>("class_list")?
                .unwrap_or_default(),
            halign: table.get("halign")?,
            valign: table.get("valign")?,
            hexpand: table.get::<Option<bool>>("hexpand")?.unwrap_or(false),
            vexpand: table.get::<Option<bool>>("vexpand")?.unwrap_or(false),
        })
    }

    pub fn apply(&self, widget: &impl IsA<gtk4::Widget>) {
        if let Some(id) = &self.id {
            widget.set_widget_name(id);
        }

        for class in &self.class_list {
            widget.add_css_class(class);
        }

        if let Some(halign) = self.halign {
            widget.set_halign(halign.into());
        }

        if let Some(valign) = self.valign {
            widget.set_valign(valign.into());
        }

        widget.set_hexpand(self.hexpand);
        widget.set_vexpand(self.vexpand);
    }
}
