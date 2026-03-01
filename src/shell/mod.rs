//! This module handles the application shell, including GTK initialization, window management, and
//! monitor-specific configuration.

pub mod config;
pub mod gtk_bindings;
mod style;

use std::rc::Rc;

use gtk4::{
    Application, ApplicationWindow, gdk,
    gio::prelude::{ApplicationExt, ApplicationExtManual, ListModelExt},
    glib,
    glib::ExitCode,
    prelude::{Cast, DisplayExt, GtkWindowExt, MonitorExt},
};
use gtk4_layer_shell::{Edge, LayerShell};

use config::Config;

/// Initializes and runs the GTK application.
///
/// This function:
/// 1. Creates the GTK Application with a unique ID based on the config title.
/// 2. Loads the CSS style if provided.
/// 3. Sets up window management for all detected monitors.
/// 4. Handles dynamic monitor hotplugging.
/// 5. Runs the GTK main loop.
///
/// Returns the application's exit code when it finishes.
pub fn run_app(config: Config) -> ExitCode {
    let app = Application::builder()
        .application_id(format!(
            "com.github.semanavasco.{}",
            config.title.to_lowercase().replace(" ", "-")
        ))
        .build();

    // Load the style on startup, before any windows are created
    let style_path = config.style.clone();
    app.connect_startup(move |_| {
        if let Some(style_path) = &style_path {
            tracing::info!("Loading style from {}", style_path);

            if let Err(e) = style::load(style_path) {
                tracing::error!("Failed to load style: {}", e);
            }
        }
    });

    // Wrap the config in an Rc so it can be shared across closures without cloning
    let config = Rc::new(config);
    app.connect_activate(move |app| {
        // Start the window manager event listener if corresponding features are enabled
        #[cfg(any(feature = "hyprland"))]
        crate::modules::wm::start_listener();

        let display = match gdk::Display::default() {
            Some(d) => d,
            None => {
                tracing::error!("No GDK display found");
                return;
            }
        };

        let monitors = display.monitors();

        // Open a window on each monitor detected at startup
        for i in 0..monitors.n_items() {
            if let Some(monitor) = monitors
                .item(i)
                .and_then(|m: glib::Object| m.downcast::<gdk::Monitor>().ok())
            {
                open_window_for_monitor(app, &config, &monitor);
            }
        }

        // React to monitors being added or removed while the app is running
        //
        // Removed items have already left the list by the time this fires; their windows are
        // closed via `connect_invalidate` registered per-window
        let app_clone = app.clone();
        let config_clone = Rc::clone(&config);
        monitors.connect_items_changed(move |list: &gtk4::gio::ListModel, pos, _removed, added| {
            for i in pos..(pos + added) {
                if let Some(monitor) = list
                    .item(i)
                    .and_then(|m: glib::Object| m.downcast::<gdk::Monitor>().ok())
                {
                    open_window_for_monitor(&app_clone, &config_clone, &monitor);
                }
            }
        });
    });

    app.run_with_args::<&str>(&[])
}

/// Opens a new window on the specified monitor if it matches the config criteria.
fn open_window_for_monitor(app: &Application, config: &Config, monitor: &gdk::Monitor) {
    let connector = monitor
        .connector()
        .map(|s: glib::GString| s.to_string())
        .unwrap_or_else(|| "unknown".to_string());

    if !config.monitors.is_empty() && !config.monitors.contains(&connector) {
        return;
    }

    tracing::info!("Opening window on monitor: {}", connector);

    let window = build_window(app, config, monitor);

    match config.child.build() {
        Ok(child) => window.set_child(Some(&child)),
        Err(e) => tracing::error!("Failed to build child widget: {}", e),
    }

    // Close this window when its monitor is invalidated
    let window_clone = window.clone();
    monitor.connect_invalidate(move |_| {
        tracing::info!("Monitor {} invalidated, closing window", connector);
        window_clone.close();
    });

    window.present();
}

/// Builds and configures a new application window based on the provided config and monitor.
fn build_window(app: &Application, config: &Config, monitor: &gdk::Monitor) -> ApplicationWindow {
    let window = ApplicationWindow::builder()
        .application(app)
        .title(config.title.clone())
        .build();

    window.init_layer_shell();
    window.set_monitor(Some(monitor));

    if config.exclusive_zone {
        window.auto_exclusive_zone_enable();
    }

    window.set_layer(config.layer.into());

    if let Some(anchors) = &config.anchors {
        let anchor_states = [
            (Edge::Top, anchors.top),
            (Edge::Bottom, anchors.bottom),
            (Edge::Left, anchors.left),
            (Edge::Right, anchors.right),
        ];
        for (edge, state) in anchor_states {
            window.set_anchor(edge, state);
        }
    }

    if let Some(margins) = &config.margins {
        let margin_states = [
            (Edge::Top, margins.top),
            (Edge::Bottom, margins.bottom),
            (Edge::Left, margins.left),
            (Edge::Right, margins.right),
        ];
        for (edge, state) in margin_states {
            window.set_margin(edge, state);
        }
    }

    window
}
