use async_channel::Receiver;
use gtk4::glib;
use mlua::{IntoLua, Value as LuaValue};

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
            let Some(lua) = LUA.get() else { continue };

            let signal_name = format!("{wm_name}::{event_type}");
            let lua_data = event_data.into_lua(lua).unwrap_or_else(|e| {
                tracing::error!("Failed to convert event data to Lua: {}", e);
                LuaValue::Nil
            });

            // Collect Rc handles under a short borrow then release it before calling the
            // callbacks. This prevents re-entrancy panics: a callback that calls
            // `subscribe`/`unsubscribe` needs `borrow_mut()`, which would conflict with `borrow()`
            let callbacks = SIGNAL_BUS.with(|bus| bus.borrow().callbacks_for(&signal_name));

            for cb in callbacks {
                cb(lua_data.clone());
            }
        }
    });
}

/// Starts the event listener for the configured window manager. This is called once during
/// application initialization. The listener runs in a background thread and forwards events to the
/// Lua signal bus on the GTK main thread.
/// It is called using cfg(any(feature = ...)) to only start the listener when one backend is
/// enabled.
pub fn start_listener() {
    #[cfg(feature = "hyprland")]
    dispatch_events(hyprland::start_listener(), "hyprland");
}
