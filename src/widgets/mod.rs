mod button;
mod container;
mod label;

use anyhow::Result;
use gtk4::{glib::object::IsA, prelude::WidgetExt};
use mlua::{FromLua, Lua, Value as LuaValue};

use crate::{dynamic::MaybeDynamic, shell::gtk_bindings::Alignment};

pub trait Widget {
    fn build(&self) -> Result<gtk4::Widget>;
}

impl FromLua for Box<dyn Widget> {
    fn from_lua(value: LuaValue, lua: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
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
    pub id: MaybeDynamic<Option<String>>,
    pub class_list: MaybeDynamic<Vec<String>>,
    pub halign: MaybeDynamic<Option<Alignment>>,
    pub valign: MaybeDynamic<Option<Alignment>>,
    pub hexpand: MaybeDynamic<bool>,
    pub vexpand: MaybeDynamic<bool>,
}

impl Properties {
    fn parse(table: &mlua::Table) -> mlua::Result<Self> {
        Ok(Properties {
            id: table
                .get::<Option<MaybeDynamic<Option<String>>>>("id")?
                .unwrap_or(MaybeDynamic::Static(None)),
            class_list: table
                .get::<Option<MaybeDynamic<Vec<String>>>>("class_list")?
                .unwrap_or(MaybeDynamic::Static(Vec::new())),
            halign: table
                .get::<Option<MaybeDynamic<Option<Alignment>>>>("halign")?
                .unwrap_or(MaybeDynamic::Static(None)),
            valign: table
                .get::<Option<MaybeDynamic<Option<Alignment>>>>("valign")?
                .unwrap_or(MaybeDynamic::Static(None)),
            hexpand: table
                .get::<Option<MaybeDynamic<bool>>>("hexpand")?
                .unwrap_or(MaybeDynamic::Static(false)),
            vexpand: table
                .get::<Option<MaybeDynamic<bool>>>("vexpand")?
                .unwrap_or(MaybeDynamic::Static(false)),
        })
    }

    pub fn apply(&self, widget: &impl IsA<gtk4::Widget>) -> Result<()> {
        let widget = widget.as_ref();

        self.id.bind(widget, "id", |w, id| {
            if let Some(id) = id {
                w.set_widget_name(&id);
            }
        })?;

        self.class_list.bind(widget, "class_list", |w, classes| {
            let class_refs: Vec<&str> = classes.iter().map(|s| s.as_str()).collect();
            w.set_css_classes(&class_refs);
        })?;

        self.halign.bind(widget, "halign", |w, halign| {
            if let Some(halign) = halign {
                w.set_halign(halign.into());
            }
        })?;

        self.valign.bind(widget, "valign", |w, valign| {
            if let Some(valign) = valign {
                w.set_valign(valign.into());
            }
        })?;

        self.hexpand.bind(widget, "hexpand", |w, hexpand| {
            w.set_hexpand(hexpand);
        })?;

        self.vexpand.bind(widget, "vexpand", |w, vexpand| {
            w.set_vexpand(vexpand);
        })?;

        Ok(())
    }
}
