mod button;
use button::Button;
mod container;
use container::Container;
mod label;
use label::Label;

use crate::{
    dynamic::MaybeReactive,
    lua::{stubs::LuaType, types::Alignment},
};
use anyhow::Result;
use gtk4::{glib::object::IsA, prelude::WidgetExt};
use mlua::{FromLua, Lua, Value as LuaValue};
use std::borrow::Cow;
use wayglance_macros::LuaClass;

/// Base trait for all UI components in wayglance.
pub trait Widget {
    /// Builds the corresponding GTK widget.
    ///
    /// # Errors
    /// Returns an error if evaluating a dynamic property fails, or if the GTK widget fails to
    /// initialize for some reason.
    fn build(&self) -> Result<gtk4::Widget>;
}

impl LuaType for Box<dyn Widget> {
    fn lua_type() -> Cow<'static, str> {
        "Widget".into()
    }
}

impl FromLua for Box<dyn Widget> {
    /// Deserializes a Lua table into a specific `Widget` trait object.
    /// The table must contain a `type` field (e.g., "button", "label") to determine which widget
    /// to instantiate.
    /// The rest of the fields are passed to the specific widget's `from_lua` implementation.
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
                let button = Button::from_lua(value, lua)?;
                Ok(Box::new(button))
            }
            "container" => {
                let container = Container::from_lua(value, lua)?;
                Ok(Box::new(container))
            }
            "label" => {
                let label = Label::from_lua(value, lua)?;
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

/// Common properties shared by all widgets (layout, CSS classes, IDs, etc).
#[derive(LuaClass)]
#[lua_class(name = "Widget")]
pub struct Properties {
    /// Optional widget ID, used for CSS styling and querying.
    pub id: MaybeReactive<Option<String>>,
    /// Optional list of CSS classes applied to the widget.
    #[lua_attr(optional)]
    pub class_list: MaybeReactive<Vec<String>>,
    /// Optional horizontal alignment for the widget.
    pub halign: MaybeReactive<Option<Alignment>>,
    /// Optional vertical alignment for the widget.
    pub valign: MaybeReactive<Option<Alignment>>,
    /// Whether the widget should expand to fill available horizontal space.
    #[lua_attr(default = false)]
    pub hexpand: MaybeReactive<bool>,
    /// Whether the widget should expand to fill available vertical space.
    #[lua_attr(default = false)]
    pub vexpand: MaybeReactive<bool>,
    /// Whether the widget is visible.
    #[lua_attr(default = true)]
    pub visible: MaybeReactive<bool>,
    /// Optional tooltip markup text for the widget.
    pub tooltip: MaybeReactive<Option<String>>,
}

impl Properties {
    /// Parses properties from a Lua table.
    ///
    /// Used turbofish syntax extensively to provide defaults for all properties without crashing
    /// if they are missing from the Lua table but still crashing if they are of the wrong type.
    fn parse(table: &mlua::Table) -> mlua::Result<Self> {
        Ok(Properties {
            id: table
                .get::<Option<MaybeReactive<Option<String>>>>("id")?
                .unwrap_or(MaybeReactive::Static(None)),
            class_list: table
                .get::<Option<MaybeReactive<Vec<String>>>>("class_list")?
                .unwrap_or(MaybeReactive::Static(Vec::new())),
            halign: table
                .get::<Option<MaybeReactive<Option<Alignment>>>>("halign")?
                .unwrap_or(MaybeReactive::Static(None)),
            valign: table
                .get::<Option<MaybeReactive<Option<Alignment>>>>("valign")?
                .unwrap_or(MaybeReactive::Static(None)),
            hexpand: table
                .get::<Option<MaybeReactive<bool>>>("hexpand")?
                .unwrap_or(MaybeReactive::Static(false)),
            vexpand: table
                .get::<Option<MaybeReactive<bool>>>("vexpand")?
                .unwrap_or(MaybeReactive::Static(false)),
            visible: table
                .get::<Option<MaybeReactive<bool>>>("visible")?
                .unwrap_or(MaybeReactive::Static(true)),
            tooltip: table
                .get::<Option<MaybeReactive<Option<String>>>>("tooltip")?
                .unwrap_or(MaybeReactive::Static(None)),
        })
    }

    /// Applies the properties to a given GTK widget.
    /// If a property is dynamic, this method automatically registers the necessary background
    /// loops and event listeners to keep the widget updated.
    ///
    /// # Errors
    /// Returns an error if evaluating a dynamic property fails, or if the GTK widget fails to
    /// update for some reason.
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

        self.visible.bind(widget, "visible", |w, visible| {
            w.set_visible(visible);
        })?;

        self.tooltip.bind(widget, "tooltip", |w, tooltip| {
            w.set_tooltip_markup(tooltip.as_deref());
        })?;

        Ok(())
    }
}
