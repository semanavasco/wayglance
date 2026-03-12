use super::Widget;
use crate::{dynamic::MaybeReactive, shell::get_config_dir, widgets::Properties};
use anyhow::Result;
use gtk4::Picture;
use mlua::FromLua;
use std::path::PathBuf;
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A widget that displays an image from a file path.
#[derive(LuaClass, WidgetBuilder)]
pub struct Image {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The file path to the image to display.
    pub src: MaybeReactive<String>,
    /// The alternative textual description for the picture.
    pub alternative_text: MaybeReactive<Option<String>>,
    /// Whether to maintain the aspect ratio of the image.
    #[lua_attr(default = true)]
    pub keep_aspect_ratio: MaybeReactive<bool>,
    /// Whether the image can be shrunk to smaller than its original size.
    #[lua_attr(default = true)]
    pub can_shrink: MaybeReactive<bool>,
}

impl Widget for Image {
    fn build(&self) -> Result<gtk4::Widget> {
        let image = Picture::new();
        self.properties.apply(&image)?;

        self.src.bind(&image, "filename", |w, src| {
            let mut path = PathBuf::from(src);

            if !path.is_absolute()
                && let Ok(config_dir) = get_config_dir()
            {
                path = config_dir.join(path);
            }

            if !path.exists() {
                tracing::warn!("Image file not found: {}", path.display());
            }

            w.set_filename(Some(path));
        })?;

        self.alternative_text
            .bind(&image, "alternative-text", |w, alt| {
                w.set_alternative_text(alt.as_deref());
            })?;

        self.keep_aspect_ratio
            .bind(&image, "keep-aspect-ratio", |w, keep| {
                w.set_keep_aspect_ratio(keep);
            })?;

        self.can_shrink.bind(&image, "can-shrink", |w, shrink| {
            w.set_can_shrink(shrink);
        })?;

        Ok(image.into())
    }
}

impl FromLua for Image {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let table = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Image".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Image {
            properties: Properties::parse(table)?,
            src: table.get("src")?,
            alternative_text: table
                .get::<Option<MaybeReactive<Option<String>>>>("alternative_text")?
                .unwrap_or(MaybeReactive::Static(None)),
            keep_aspect_ratio: table
                .get::<Option<MaybeReactive<bool>>>("keep_aspect_ratio")?
                .unwrap_or(MaybeReactive::Static(true)),
            can_shrink: table
                .get::<Option<MaybeReactive<bool>>>("can_shrink")?
                .unwrap_or(MaybeReactive::Static(true)),
        })
    }
}
