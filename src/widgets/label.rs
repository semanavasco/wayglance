use std::{cell::Cell, time::Duration};

use anyhow::{Context, Result};
use gtk4::{Label as GtkLabel, glib, prelude::WidgetExt};
use mlua::FromLua;

use crate::{
    dynamic::MaybeDynamic,
    shell::config::LUA,
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
                self.properties.apply(&label);
                Ok(label.into())
            }
            MaybeDynamic::Interval { callback, interval } => {
                let lua = LUA.get().context("Lua instance not initialized")?;
                let callback = lua.registry_value::<mlua::Function>(callback)?;

                let label = GtkLabel::new(Some(&callback.call::<String>(())?));
                self.properties.apply(&label);

                let label_clone = label.clone();
                let source_id =
                    glib::timeout_add_local(Duration::from_millis(*interval), move || {
                        match callback.call::<String>(()) {
                            Ok(text) => label_clone.set_text(&text),
                            Err(e) => {
                                tracing::error!("Error calling Lua callback for Label: {}", e)
                            }
                        }

                        glib::ControlFlow::Continue
                    });

                let source_id = Cell::new(Some(source_id));
                label.connect_destroy(move |_| {
                    if let Some(id) = source_id.take() {
                        id.remove();
                    }
                });

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
