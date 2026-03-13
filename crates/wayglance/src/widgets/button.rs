use crate::{
    lua::LUA,
    widgets::{Properties, Widget},
};
use anyhow::{Context, Result};
use gtk4::{Button as GtkButton, prelude::ButtonExt};
use mlua::{FromLua, Lua, Value as LuaValue};
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A clickable button widget.
#[derive(LuaClass, WidgetBuilder)]
#[lua_class(name = "ButtonWidget")]
struct Button {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// Function to execute when the button is clicked.
    pub on_click: mlua::RegistryKey,
    /// The child widget to display inside the button.
    pub child: Box<dyn Widget>,
}

impl Widget for Button {
    fn build(&self) -> Result<gtk4::Widget> {
        let button = GtkButton::new();

        self.properties.apply(&button)?;

        button.set_child(Some(&self.child.build()?));

        let lua = LUA.get().context("Lua instance not initialized")?;

        let function = lua
            .registry_value::<mlua::Function>(&self.on_click)
            .context("Failed to retrieve Lua function")?;

        button.connect_clicked(move |_| {
            if let Err(e) = function.call::<()>(()) {
                tracing::error!("Error calling Lua on_click function: {}", e);
            }
        });

        Ok(button.into())
    }
}

impl FromLua for Button {
    fn from_lua(value: LuaValue, lua: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Button".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        let on_click = match table.get::<LuaValue>("on_click")? {
            LuaValue::Function(func) => lua.create_registry_value(func)?,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: "non-function",
                    to: "Button on_click".to_string(),
                    message: Some("Expected a function for on_click".to_string()),
                });
            }
        };

        Ok(Button {
            properties: Properties::parse(table)?,
            on_click,
            child: table.get("child")?,
        })
    }
}
