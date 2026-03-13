use super::Widget;
use crate::{dynamic::MaybeReactive, widgets::Properties};
use anyhow::Result;
use gtk4::Image as GtkImage;
use mlua::FromLua;
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A widget that displays a GTK icon.
#[derive(LuaClass, WidgetBuilder)]
#[lua_class(name = "IconWidget")]
pub struct Icon {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The name of the icon to display (e.g. "audio-volume-high").
    pub name: MaybeReactive<String>,
    /// The size of the icon in pixels.
    #[lua_attr(default = 24)]
    pub size: MaybeReactive<i32>,
    /// Whether to use a fallback icon if the specified icon name is not found.
    #[lua_attr(default = true)]
    pub use_fallback: MaybeReactive<bool>,
}

impl Widget for Icon {
    fn build(&self) -> Result<gtk4::Widget> {
        let icon = GtkImage::new();
        self.properties.apply(&icon)?;

        self.name.bind(&icon, "name", |w, name| {
            w.set_icon_name(Some(&name));
        })?;

        self.size.bind(&icon, "pixel-size", |w, size| {
            w.set_pixel_size(size);
        })?;

        self.use_fallback
            .bind(&icon, "use-fallback", |w, use_fallback| {
                w.set_use_fallback(use_fallback);
            })?;

        Ok(icon.into())
    }
}

impl FromLua for Icon {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let table = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Icon".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Icon {
            properties: Properties::parse(table)?,
            name: table.get("name")?,
            size: table
                .get::<Option<MaybeReactive<i32>>>("size")?
                .unwrap_or(MaybeReactive::Static(24)),
            use_fallback: table
                .get::<Option<MaybeReactive<bool>>>("use_fallback")?
                .unwrap_or(MaybeReactive::Static(true)),
        })
    }
}
