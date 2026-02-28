use std::thread;

use async_channel::Receiver;
use hyprland::{event_listener::EventListener, shared::WorkspaceType};
use mlua::{IntoLua, Lua, Value as LuaValue};

/// Events emitted by the Hyprland IPC listener.
///
/// Each variant maps to Lua signal(s) dispatched as `hyprland::<event_name>`.
pub enum HyprlandEvent {
    /// Emitted for workspace lifecycle and focus events.
    ///
    /// Signals:
    /// - `hyprland::workspace_changed` : the user switched to a different workspace
    /// - `hyprland::workspace_added`   : a new workspace was created
    /// - `hyprland::workspace_deleted` : a workspace was destroyed
    /// - `hyprland::workspace_moved`   : a workspace was moved to another monitor
    /// - `hyprland::workspace_renamed` : a workspace was given a new name
    Workspace { id: i32, name: String },

    /// Emitted when the focused window changes.
    ///
    /// Signal: `hyprland::active_window`
    ///
    /// Both fields are empty strings when no window is focused (e.g. on an empty workspace).
    ActiveWindowChanged { title: String, class: String },

    /// Emitted when the fullscreen state of the active window toggles.
    ///
    /// Signal: `hyprland::fullscreen_changed`
    FullscreenStateChanged(bool),

    /// Emitted when keyboard focus moves to a different monitor.
    ///
    /// Signal: `hyprland::active_monitor_changed`
    ///
    /// `workspace` is `nil` in Lua when Hyprland does not report an associated
    /// workspace for the monitor at the time of the event.
    ActiveMonitorChanged {
        monitor: String,
        workspace: Option<String>,
    },
}

impl IntoLua for HyprlandEvent {
    fn into_lua(self, lua: &Lua) -> mlua::Result<LuaValue> {
        match self {
            HyprlandEvent::Workspace { id, name } => {
                let table = lua.create_table()?;
                table.set("id", id)?;
                table.set("name", name)?;
                Ok(LuaValue::Table(table))
            }
            HyprlandEvent::ActiveWindowChanged { title, class } => {
                let table = lua.create_table()?;
                table.set("title", title)?;
                table.set("class", class)?;
                Ok(LuaValue::Table(table))
            }
            HyprlandEvent::FullscreenStateChanged(state) => Ok(LuaValue::Boolean(state)),
            HyprlandEvent::ActiveMonitorChanged { monitor, workspace } => {
                let table = lua.create_table()?;
                table.set("monitor", monitor)?;
                if let Some(ws) = workspace {
                    table.set("workspace", ws)?;
                }
                Ok(LuaValue::Table(table))
            }
        }
    }
}

/// Spawns a background thread that connects to the Hyprland IPC socket and listens for compositor
/// events.
///
/// Returns a [`Receiver`] that yields `(signal_name, event)` pairs. The `signal_name` corresponds
/// to the `hyprland::<name>` signal emitted on the Lua signal bus. Callers must drive the receiver
/// on the GTK main thread and forward each event to the signal bus.
///
/// The thread runs until the Hyprland IPC socket closes or an unrecoverable error occurs, at which
/// point an error is logged and the thread exits.
pub fn start_listener() -> Receiver<(String, HyprlandEvent)> {
    let (sender, receiver) = async_channel::unbounded();

    thread::spawn(move || {
        let mut listener = EventListener::new();

        let sender_clone = sender.clone();
        listener.add_workspace_changed_handler(move |workspace| {
            let ws_name = match workspace.name {
                WorkspaceType::Regular(name) => name,
                WorkspaceType::Special(Some(name)) => name,
                WorkspaceType::Special(None) => "special".to_string(),
            };

            if sender_clone
                .send_blocking((
                    "workspace_changed".to_string(),
                    HyprlandEvent::Workspace {
                        id: workspace.id,
                        name: ws_name,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send workspace change event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_workspace_deleted_handler(move |workspace| {
            let ws_name = match workspace.name {
                WorkspaceType::Regular(name) => name,
                WorkspaceType::Special(Some(name)) => name,
                WorkspaceType::Special(None) => "special".to_string(),
            };

            if sender_clone
                .send_blocking((
                    "workspace_deleted".to_string(),
                    HyprlandEvent::Workspace {
                        id: workspace.id,
                        name: ws_name,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send workspace destroy event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_workspace_added_handler(move |workspace| {
            let ws_name = match workspace.name {
                WorkspaceType::Regular(name) => name,
                WorkspaceType::Special(Some(name)) => name,
                WorkspaceType::Special(None) => "special".to_string(),
            };

            if sender_clone
                .send_blocking((
                    "workspace_added".to_string(),
                    HyprlandEvent::Workspace {
                        id: workspace.id,
                        name: ws_name,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send workspace added event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_workspace_moved_handler(move |workspace| {
            let ws_name = match workspace.name {
                WorkspaceType::Regular(name) => name,
                WorkspaceType::Special(Some(name)) => name,
                WorkspaceType::Special(None) => "special".to_string(),
            };

            if sender_clone
                .send_blocking((
                    "workspace_moved".to_string(),
                    HyprlandEvent::Workspace {
                        id: workspace.id,
                        name: ws_name,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send workspace moved event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_workspace_renamed_handler(move |workspace| {
            if sender_clone
                .send_blocking((
                    "workspace_renamed".to_string(),
                    HyprlandEvent::Workspace {
                        id: workspace.id,
                        name: workspace.name,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send workspace renamed event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_active_window_changed_handler(move |event| {
            let (title, class) = match event {
                Some(window) => (window.title, window.class),
                None => (String::new(), String::new()),
            };

            if sender_clone
                .send_blocking((
                    "active_window".to_string(),
                    HyprlandEvent::ActiveWindowChanged { title, class },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send active_window change event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_fullscreen_state_changed_handler(move |state| {
            if sender_clone
                .send_blocking((
                    "fullscreen_changed".to_string(),
                    HyprlandEvent::FullscreenStateChanged(state),
                ))
                .is_err()
            {
                tracing::warn!("Failed to send fullscreen_changed event");
            }
        });

        let sender_clone = sender.clone();
        listener.add_active_monitor_changed_handler(move |event| {
            let workspace = event.workspace_name.map(|ws| match ws {
                WorkspaceType::Regular(name) => name,
                WorkspaceType::Special(Some(name)) => name,
                WorkspaceType::Special(None) => "special".to_string(),
            });

            if sender_clone
                .send_blocking((
                    "active_monitor_changed".to_string(),
                    HyprlandEvent::ActiveMonitorChanged {
                        monitor: event.monitor_name,
                        workspace,
                    },
                ))
                .is_err()
            {
                tracing::warn!("Failed to send active_monitor_changed event");
            }
        });

        tracing::debug!("Starting Hyprland IPC listener...");

        if let Err(e) = listener.start_listener() {
            tracing::error!("Hyprland event listener crashed: {}", e);
        }
    });

    receiver
}
