use async_channel::Receiver;
use gtk4::glib;
use mlua::{IntoLua, Lua, Value as LuaValue};

use crate::dynamic::SIGNAL_BUS;
use crate::shell::config::LUA;

#[cfg(feature = "hyprland")]
mod hyprland;

/// Drives a WM event receiver on the GTK main context, forwarding every event
/// to the Lua signal bus as `<wm_name>::<event_type>`.
///
/// `T` must implement [`IntoLua`] so the data can be handed to Lua callbacks. Each
/// backend (Hyprland, Sway, …) produces its own `T` and its own receiver.
fn dispatch_events<T>(receiver: Receiver<(String, T)>, wm_name: &'static str)
where
    T: IntoLua + 'static,
{
    glib::MainContext::default().spawn_local(async move {
        while let Ok((event_type, event_data)) = receiver.recv().await {
            SIGNAL_BUS.with(|bus| {
                let signal_name = format!("{wm_name}::{event_type}");

                LUA.get().map(|lua| {
                    bus.borrow().emit(
                        &signal_name,
                        event_data.into_lua(lua).unwrap_or_else(|e| {
                            tracing::error!("Failed to convert event data to Lua: {}", e);
                            LuaValue::Nil
                        }),
                    );
                });
            });
        }
    });
}

pub fn start_listener() {
    #[cfg(feature = "hyprland")]
    dispatch_events(hyprland::start_listener(), "hyprland");
}
