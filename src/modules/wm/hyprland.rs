use std::thread;

use async_channel::Receiver;
use hyprland::{
    data::{Client, Monitors, Workspaces},
    dispatch::{Dispatch, DispatchType, FullscreenType, WorkspaceIdentifierWithSpecial},
    event_listener::EventListener,
    shared::{HyprData, HyprDataActiveOptional, HyprDataVec, WorkspaceType},
};
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

/// Helper function to call a dispatch and convert any errors into an mlua::Error with a
/// descriptive message.
fn call_dispatch(dispatch_type: DispatchType<'_>, action: &str) -> mlua::Result<()> {
    Dispatch::call(dispatch_type)
        .map_err(|e| mlua::Error::external(format!("Failed to {}: {}", action, e)))
}

/// Registers Hyprland-specific Lua functions under the `hyprland` table.
pub fn register_lua(lua: &Lua, table: &mlua::Table) -> mlua::Result<()> {
    let hyprland = lua.create_table()?;

    hyprland.set(
        "getWorkspaces",
        lua.create_function(|lua, ()| {
            let workspaces = Workspaces::get()
                .map_err(|e| mlua::Error::external(format!("Failed to get workspaces: {}", e)))?
                .to_vec();

            let ws_table = lua.create_table()?;

            for (i, ws) in workspaces.into_iter().enumerate() {
                let ws_entry = lua.create_table()?;

                ws_entry.set("id", ws.id)?;
                ws_entry.set("name", ws.name)?;
                ws_entry.set("monitor", ws.monitor)?;
                ws_entry.set("windows", ws.windows)?;
                ws_entry.set("last_window_title", ws.last_window_title)?;
                ws_entry.set("fullscreen", ws.fullscreen)?;
                ws_entry.set("monitor_id", ws.monitor_id)?;

                ws_table.set(i + 1, ws_entry)?;
            }

            Ok(ws_table)
        })?,
    )?;

    hyprland.set(
        "getMonitors",
        lua.create_function(|lua, ()| {
            let monitors = Monitors::get()
                .map_err(|e| mlua::Error::external(format!("Failed to get monitors: {}", e)))?
                .to_vec();

            let monitor_table = lua.create_table()?;

            for (i, monitor) in monitors.into_iter().enumerate() {
                let monitor_entry = lua.create_table()?;

                monitor_entry.set("id", monitor.id)?;
                monitor_entry.set("name", monitor.name)?;
                monitor_entry.set("focused", monitor.focused)?;

                monitor_table.set(i + 1, monitor_entry)?;
            }

            Ok(monitor_table)
        })?,
    )?;

    hyprland.set(
        "getActiveWindow",
        lua.create_function(|lua, ()| {
            let active_window = Client::get_active().map_err(|e| {
                mlua::Error::external(format!("Failed to get active window: {}", e))
            })?;

            let window_table = lua.create_table()?;

            if let Some(window) = active_window {
                window_table.set("title", window.title)?;
                window_table.set("class", window.class)?;
                window_table.set("pid", window.pid)?;
                window_table.set("monitor", window.monitor)?;

                let workspace = lua.create_table()?;
                workspace.set("id", window.workspace.id)?;
                workspace.set("name", window.workspace.name)?;
                window_table.set("workspace", workspace)?;

                let at = lua.create_table()?;
                at.set("x", window.at.0)?;
                at.set("y", window.at.1)?;
                window_table.set("at", at)?;

                let size = lua.create_table()?;
                size.set("width", window.size.0)?;
                size.set("height", window.size.1)?;
                window_table.set("size", size)?;
            }

            Ok(window_table)
        })?,
    )?;

    hyprland.set(
        "switchWorkspace",
        lua.create_function(|_, workspace_id: i32| {
            call_dispatch(
                DispatchType::Workspace(WorkspaceIdentifierWithSpecial::Id(workspace_id)),
                "switch workspace",
            )
        })?,
    )?;

    hyprland.set(
        "switchWorkspaceRelative",
        lua.create_function(|_, offset: i32| {
            call_dispatch(
                DispatchType::Workspace(WorkspaceIdentifierWithSpecial::Relative(offset)),
                "switch workspace",
            )
        })?,
    )?;

    hyprland.set(
        "switchWorkspaceNamed",
        lua.create_function(|_, workspace_name: String| {
            call_dispatch(
                DispatchType::Workspace(WorkspaceIdentifierWithSpecial::Name(&workspace_name)),
                "switch workspace",
            )
        })?,
    )?;

    hyprland.set(
        "switchToPreviousWorkspace",
        lua.create_function(|_, ()| {
            call_dispatch(
                DispatchType::Workspace(WorkspaceIdentifierWithSpecial::Previous),
                "switch workspace",
            )
        })?,
    )?;

    hyprland.set(
        "moveActiveToWorkspace",
        lua.create_function(|_, workspace_id: i32| {
            call_dispatch(
                DispatchType::MoveToWorkspace(
                    WorkspaceIdentifierWithSpecial::Id(workspace_id),
                    None,
                ),
                "move active window to workspace",
            )
        })?,
    )?;

    hyprland.set(
        "moveActiveToWorkspaceSilent",
        lua.create_function(|_, workspace_id: i32| {
            call_dispatch(
                DispatchType::MoveToWorkspaceSilent(
                    WorkspaceIdentifierWithSpecial::Id(workspace_id),
                    None,
                ),
                "move active window to workspace silently",
            )
        })?,
    )?;

    hyprland.set(
        "toggleSpecialWorkspace",
        lua.create_function(|_, workspace_name: Option<String>| {
            call_dispatch(
                DispatchType::ToggleSpecialWorkspace(workspace_name),
                "toggle special workspace",
            )
        })?,
    )?;

    hyprland.set(
        "toggleFloating",
        lua.create_function(|_, ()| {
            call_dispatch(DispatchType::ToggleFloating(None), "toggle floating")
        })?,
    )?;

    hyprland.set(
        "toggleFullscreen",
        lua.create_function(|_, ()| {
            call_dispatch(
                DispatchType::ToggleFullscreen(FullscreenType::NoParam),
                "toggle fullscreen",
            )
        })?,
    )?;

    hyprland.set(
        "killActiveWindow",
        lua.create_function(|_, ()| {
            call_dispatch(DispatchType::KillActiveWindow, "kill active window")
        })?,
    )?;

    table.set("hyprland", hyprland)?;
    Ok(())
}
