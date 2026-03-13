use crate::{
    dynamic::{MaybeReactive, state::State},
    lua::{LUA, types::Orientation},
    widgets::{Properties, Widget},
};
use anyhow::{Context, Result};
use gtk4::{
    Box as GtkBox,
    prelude::{BoxExt, WidgetExt},
};
use mlua::{FromLua, Lua, Value as LuaValue};
use std::rc::Rc;
use wayglance_macros::{LuaClass, WidgetBuilder};

/// A container widget that can hold multiple child widgets, arranged either horizontally or
/// vertically.
#[derive(LuaClass, WidgetBuilder)]
#[lua_class(name = "ContainerWidget")]
struct Container {
    #[lua_attr(parent)]
    pub properties: Properties,
    /// The orientation of the container.
    pub orientation: Orientation,
    /// The spacing between children in the container, in pixels.
    #[lua_attr(default = 0)]
    pub spacing: i32,
    /// The child widgets contained within this container.
    #[lua_attr(optional)]
    pub children: MaybeReactive<Vec<Box<dyn Widget>>>,
}

impl Widget for Container {
    fn build(&self) -> Result<gtk4::Widget> {
        let container = GtkBox::new(self.orientation.into(), self.spacing);

        self.properties.apply(&container)?;

        match &self.children {
            MaybeReactive::Static(children) => {
                for child in children {
                    container.append(&child.build()?);
                }
            }
            MaybeReactive::Bound(state) => {
                let lua = LUA.get().context("Lua instance not initialized")?;

                // Build initial children from state
                let current = state.get(lua)?;
                if let Ok(children) = Vec::<Box<dyn Widget>>::from_lua(current, lua) {
                    for child in &children {
                        container.append(&child.build()?);
                    }
                }

                // Subscribe for future updates: rebuild all children when state changes
                // TODO: optimize by diffing old vs new children and only updating changed ones
                let transform_fn = state
                    .transform
                    .as_ref()
                    .map(|k| lua.registry_value::<mlua::Function>(k))
                    .transpose()?;

                let container_clone = container.clone();
                let subscriber: Rc<dyn Fn(LuaValue)> = Rc::new(move |value: LuaValue| {
                    let Some(lua) = LUA.get() else { return };

                    let resolved = if let Some(ref func) = transform_fn {
                        match func.call::<LuaValue>(value) {
                            Ok(v) => v,
                            Err(e) => {
                                tracing::error!("Error in container children transform: {}", e);
                                return;
                            }
                        }
                    } else {
                        value
                    };

                    match Vec::<Box<dyn Widget>>::from_lua(resolved, lua) {
                        Ok(children) => {
                            while let Some(child) = container_clone.first_child() {
                                container_clone.remove(&child);
                            }
                            for child in &children {
                                match child.build() {
                                    Ok(w) => container_clone.append(&w),
                                    Err(e) => {
                                        tracing::error!("Error building dynamic child: {}", e)
                                    }
                                }
                            }
                        }
                        Err(e) => {
                            tracing::error!("Error parsing dynamic children: {}", e);
                        }
                    }
                });

                let state_id = state.id;
                if let Some(sub_id) = state.subscribe(subscriber) {
                    container.connect_destroy(move |_| {
                        State::unsubscribe(state_id, sub_id);
                    });
                }
            }
        }

        Ok(container.into())
    }
}

impl FromLua for Container {
    fn from_lua(value: LuaValue, _: &Lua) -> mlua::Result<Self> {
        let table = match &value {
            LuaValue::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Container".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Container {
            properties: Properties::parse(table)?,
            orientation: table.get("orientation")?,
            spacing: table.get::<Option<i32>>("spacing")?.unwrap_or(0),
            children: table
                .get::<Option<MaybeReactive<Vec<Box<dyn Widget>>>>>("children")?
                .unwrap_or(MaybeReactive::Static(Vec::new())),
        })
    }
}
