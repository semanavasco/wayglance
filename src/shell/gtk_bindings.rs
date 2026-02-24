use gtk4::{Align as GtkAlign, Orientation as GtkOrientation};
use gtk4_layer_shell::Layer as GtkLayer;
use mlua::FromLua;

#[derive(Clone, Copy)]
pub enum Layer {
    Background,
    Bottom,
    Top,
    Overlay,
}

impl FromLua for Layer {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let layer = match &value {
            mlua::Value::String(s) => s.to_str()?,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Layer".to_string(),
                    message: Some("Expected a string".to_string()),
                });
            }
        };

        match layer.to_lowercase().as_str() {
            "background" => Ok(Layer::Background),
            "bottom" => Ok(Layer::Bottom),
            "top" => Ok(Layer::Top),
            "overlay" => Ok(Layer::Overlay),
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to: "Layer".to_string(),
                message: Some(format!("Invalid layer: {}", layer)),
            }),
        }
    }
}

impl From<Layer> for GtkLayer {
    fn from(value: Layer) -> Self {
        match value {
            Layer::Background => GtkLayer::Background,
            Layer::Bottom => GtkLayer::Bottom,
            Layer::Top => GtkLayer::Top,
            Layer::Overlay => GtkLayer::Overlay,
        }
    }
}

pub struct Anchors {
    pub top: bool,
    pub right: bool,
    pub bottom: bool,
    pub left: bool,
}

impl FromLua for Anchors {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let anchors = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Anchors".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Anchors {
            top: anchors.get::<Option<bool>>("top")?.unwrap_or(false),
            right: anchors.get::<Option<bool>>("right")?.unwrap_or(false),
            bottom: anchors.get::<Option<bool>>("bottom")?.unwrap_or(false),
            left: anchors.get::<Option<bool>>("left")?.unwrap_or(false),
        })
    }
}

pub struct Margins {
    pub top: i32,
    pub right: i32,
    pub bottom: i32,
    pub left: i32,
}

impl FromLua for Margins {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let margins = match &value {
            mlua::Value::Table(t) => t,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Margins".to_string(),
                    message: Some("Expected a table".to_string()),
                });
            }
        };

        Ok(Margins {
            top: margins.get::<Option<i32>>("top")?.unwrap_or(0),
            right: margins.get::<Option<i32>>("right")?.unwrap_or(0),
            bottom: margins.get::<Option<i32>>("bottom")?.unwrap_or(0),
            left: margins.get::<Option<i32>>("left")?.unwrap_or(0),
        })
    }
}

#[derive(Clone, Copy)]
pub enum Orientation {
    Horizontal,
    Vertical,
}

impl FromLua for Orientation {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let orientation = match &value {
            mlua::Value::String(s) => s.to_str()?,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Orientation".to_string(),
                    message: Some("Expected a string".to_string()),
                });
            }
        };

        match orientation.to_lowercase().as_str() {
            "horizontal" => Ok(Orientation::Horizontal),
            "vertical" => Ok(Orientation::Vertical),
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to: "Orientation".to_string(),
                message: Some(format!("Invalid orientation: {}", orientation)),
            }),
        }
    }
}

impl From<Orientation> for GtkOrientation {
    fn from(value: Orientation) -> Self {
        match value {
            Orientation::Horizontal => GtkOrientation::Horizontal,
            Orientation::Vertical => GtkOrientation::Vertical,
        }
    }
}

#[derive(Clone, Copy)]
pub enum Alignment {
    Start,
    Center,
    End,
    Fill,
    Baseline,
}

impl FromLua for Alignment {
    fn from_lua(value: mlua::Value, _: &mlua::Lua) -> mlua::Result<Self> {
        let alignment = match &value {
            mlua::Value::String(s) => s.to_str()?,
            _ => {
                return Err(mlua::Error::FromLuaConversionError {
                    from: value.type_name(),
                    to: "Alignment".to_string(),
                    message: Some("Expected a string".to_string()),
                });
            }
        };

        match alignment.to_lowercase().as_str() {
            "start" => Ok(Alignment::Start),
            "center" => Ok(Alignment::Center),
            "end" => Ok(Alignment::End),
            "fill" => Ok(Alignment::Fill),
            "baseline" => Ok(Alignment::Baseline),
            _ => Err(mlua::Error::FromLuaConversionError {
                from: value.type_name(),
                to: "Alignment".to_string(),
                message: Some(format!("Invalid alignment: {}", alignment)),
            }),
        }
    }
}

impl From<Alignment> for GtkAlign {
    fn from(value: Alignment) -> Self {
        match value {
            Alignment::Start => GtkAlign::Start,
            Alignment::Center => GtkAlign::Center,
            Alignment::End => GtkAlign::End,
            Alignment::Fill => GtkAlign::Fill,
            Alignment::Baseline => GtkAlign::Baseline,
        }
    }
}
