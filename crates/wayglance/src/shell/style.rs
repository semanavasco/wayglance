use std::path::PathBuf;

use anyhow::{Context, Result};
use gtk4::{
    CssProvider, STYLE_PROVIDER_PRIORITY_USER, gdk::Display, style_context_add_provider_for_display,
};

use crate::shell::config::get_config_dir;

/// Loads a CSS style from the specified path and applies it to the GTK application.
/// The path can be either absolute or relative to the config file directory.
///
/// # Errors
/// Returns an error if the CSS file cannot be loaded or if there is no display available.
pub fn load(path: &str) -> Result<()> {
    let provider = CssProvider::new();

    let path = PathBuf::from(path);

    if path.is_absolute() {
        provider.load_from_path(path);
    } else {
        let path = get_config_dir()?.join(path);
        provider.load_from_path(path);
    }

    style_context_add_provider_for_display(
        &Display::default().context("Could not connect to a display")?,
        &provider,
        STYLE_PROVIDER_PRIORITY_USER,
    );

    Ok(())
}
