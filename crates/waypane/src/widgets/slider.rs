use crate::{
    dynamic::MaybeReactive,
    lua::{LUA, types::Orientation},
    widgets::{Properties, Widget},
};
use anyhow::{Context, Result};
use gtk4::prelude::{AdjustmentExt, OrientableExt, RangeExt, ScaleExt};
use gtk4::{Adjustment, Scale};
use mlua::{FromLua, Lua, Value as LuaValue};
use waypane_macros::{LuaClass, WidgetBuilder};

/// A widget that allows users to select a value from a range by sliding a handle.
#[derive(LuaClass, WidgetBuilder)]
#[lua_class(name = "SliderWidget")]
pub struct Slider {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The current value of the slider.
    #[lua_attr(default = 0.0)]
    pub value: MaybeReactive<f64>,
    /// The minimum value of the slider.
    #[lua_attr(default = 0.0)]
    pub min: MaybeReactive<f64>,
    /// The maximum value of the slider.
    #[lua_attr(default = 100.0)]
    pub max: MaybeReactive<f64>,
    /// The step increment for the slider.
    #[lua_attr(default = 1.0)]
    pub step: MaybeReactive<f64>,
    /// The page step increment for the slider.
    #[lua_attr(default = 10.0)]
    pub page_step: MaybeReactive<f64>,
    /// The number of decimal digits for the slider value. If `nil`, falls back to 1 digit.
    pub digits: MaybeReactive<Option<i32>>,
    /// Whether to draw the current value next to the slider.
    #[lua_attr(default = true)]
    pub draw_value: MaybeReactive<bool>,
    /// The orientation of the slider.
    #[lua_attr(default = "horizontal")]
    pub orientation: MaybeReactive<Orientation>,
    /// Function to execute when the slider value changes.
    pub on_change: mlua::RegistryKey,
}

impl Widget for Slider {
    fn build(&self) -> Result<gtk4::Widget> {
        let slider = Scale::new(gtk4::Orientation::Horizontal, Some(&Adjustment::default()));
        self.properties.apply(&slider)?;

        self.min.bind(&slider, "min", |w, min| {
            w.adjustment().set_lower(min);
        })?;

        self.max.bind(&slider, "max", |w, max| {
            w.adjustment().set_upper(max);
        })?;

        self.value.bind(&slider, "value", |w, value| {
            w.set_value(value);
        })?;

        self.step.bind(&slider, "step", |w, step| {
            w.adjustment().set_step_increment(step);
        })?;

        self.page_step.bind(&slider, "page_step", |w, page_step| {
            w.adjustment().set_page_increment(page_step);
        })?;

        self.digits.bind(&slider, "digits", |w, digits| {
            w.set_digits(digits.unwrap_or(1));
        })?;

        self.draw_value.bind(&slider, "draw-value", |w, draw| {
            w.set_draw_value(draw);
        })?;

        self.orientation
            .bind(&slider, "orientation", |w, orientation| {
                w.set_orientation(orientation.into());
            })?;

        let lua = LUA.get().context("Lua instance not initialized")?;

        let function = lua
            .registry_value::<mlua::Function>(&self.on_change)
            .context("Failed to retrieve Lua function")?;

        slider.connect_value_changed(move |s| {
            let value = s.value();
            if let Err(e) = function.call::<()>(value) {
                tracing::error!("Error calling Lua on_change function: {}", e);
            }
        });

        Ok(slider.into())
    }
}

impl FromLua for Slider {
    fn from_lua(value: LuaValue, lua: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Slider".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        let on_change = match table.get::<LuaValue>("on_change")? {
            LuaValue::Function(func) => lua.create_registry_value(func)?,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: "non-function",
                    to: "Slider on_change".to_string(),
                    message: Some("Expected a function for on_change".to_string()),
                });
            }
        };

        Ok(Slider {
            properties: Properties::parse(table)?,
            value: table
                .get::<Option<MaybeReactive<f64>>>("value")?
                .unwrap_or(MaybeReactive::Static(0.0)),
            min: table
                .get::<Option<MaybeReactive<f64>>>("min")?
                .unwrap_or(MaybeReactive::Static(0.0)),
            max: table
                .get::<Option<MaybeReactive<f64>>>("max")?
                .unwrap_or(MaybeReactive::Static(100.0)),
            step: table
                .get::<Option<MaybeReactive<f64>>>("step")?
                .unwrap_or(MaybeReactive::Static(1.0)),
            page_step: table
                .get::<Option<MaybeReactive<f64>>>("page_step")?
                .unwrap_or(MaybeReactive::Static(10.0)),
            digits: table
                .get::<Option<MaybeReactive<Option<i32>>>>("digits")?
                .unwrap_or(MaybeReactive::Static(None)),
            draw_value: table
                .get::<Option<MaybeReactive<bool>>>("draw_value")?
                .unwrap_or(MaybeReactive::Static(true)),
            orientation: table
                .get::<Option<MaybeReactive<Orientation>>>("orientation")?
                .unwrap_or(MaybeReactive::Static(Orientation::Horizontal)),
            on_change,
        })
    }
}
