pub mod config;
pub mod gtk_bindings;
mod style;

use anyhow::Result;
use gtk4::{
    Application, ApplicationWindow,
    gio::prelude::{ApplicationExt, ApplicationExtManual},
    glib::ExitCode,
    prelude::GtkWindowExt,
};
use gtk4_layer_shell::{Edge, LayerShell};

use config::Config;

pub fn run_app(config: Config) -> Result<ExitCode> {
    let app = Application::builder()
        .application_id(format!(
            "com.github.semanavasco.{}",
            config.title.to_lowercase().replace(" ", "-")
        ))
        .build();

    let style_path = config.style.clone();
    app.connect_startup(move |_| {
        if let Some(style_path) = &style_path {
            tracing::info!("Loading style from {}", style_path);

            if let Err(e) = style::load(style_path) {
                tracing::error!("Failed to load style: {}", e);
            }
        }
    });

    app.connect_activate(move |app| {
        let window = build_window(app, &config);

        match config.child.build() {
            Ok(child) => window.set_child(Some(&child)),
            Err(e) => tracing::error!("Failed to build child widget: {}", e),
        }

        window.present();
    });

    Ok(app.run_with_args::<&str>(&[]))
}

fn build_window(app: &Application, config: &Config) -> ApplicationWindow {
    let window = ApplicationWindow::builder()
        .application(app)
        .title(config.title.clone())
        .build();

    window.init_layer_shell();

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
