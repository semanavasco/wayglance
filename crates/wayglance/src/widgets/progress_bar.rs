use crate::{
    dynamic::MaybeReactive,
    lua::types::Orientation,
    widgets::{Properties, Widget},
};
use anyhow::Result;
use gtk4::ProgressBar as GtkProgressBar;
use gtk4::prelude::OrientableExt;
use mlua::{FromLua, Lua, Value as LuaValue};
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A widget that displays a progress bar.
#[derive(LuaClass, WidgetBuilder)]
#[lua_class(name = "ProgressBarWidget")]
pub struct ProgressBar {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The fraction of the progress bar that is filled, between 0.0 and 1.0.
    #[lua_attr(default = 0.0)]
    pub fraction: MaybeReactive<f64>,
    /// The text to display over the progress bar if provided.
    pub text: MaybeReactive<Option<String>>,
    /// Whether the progress bar is inverted.
    #[lua_attr(default = false)]
    pub inverted: MaybeReactive<bool>,
    /// The orientation of the progress bar.
    #[lua_attr(default = "horizontal")]
    pub orientation: MaybeReactive<Orientation>,
}

impl Widget for ProgressBar {
    fn build(&self) -> Result<gtk4::Widget> {
        let bar = GtkProgressBar::new();
        self.properties.apply(&bar)?;

        self.fraction.bind(&bar, "fraction", |w, fraction| {
            w.set_fraction(fraction);
        })?;

        self.text.bind(&bar, "text", |w, text| {
            w.set_show_text(text.is_some());
            w.set_text(text.as_deref());
        })?;

        self.inverted.bind(&bar, "inverted", |w, inverted| {
            w.set_inverted(inverted);
        })?;

        self.orientation
            .bind(&bar, "orientation", |w, orientation| {
                w.set_orientation(orientation.into());
            })?;

        Ok(bar.into())
    }
}

impl FromLua for ProgressBar {
    fn from_lua(value: LuaValue, _: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "ProgressBar".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(ProgressBar {
            properties: Properties::parse(table)?,
            fraction: table
                .get::<Option<MaybeReactive<f64>>>("fraction")?
                .unwrap_or(MaybeReactive::Static(0.0)),
            text: table
                .get::<Option<MaybeReactive<Option<String>>>>("text")?
                .unwrap_or(MaybeReactive::Static(None)),
            inverted: table
                .get::<Option<MaybeReactive<bool>>>("inverted")?
                .unwrap_or(MaybeReactive::Static(false)),
            orientation: table
                .get::<Option<MaybeReactive<Orientation>>>("orientation")?
                .unwrap_or(MaybeReactive::Static(Orientation::Horizontal)),
        })
    }
}
