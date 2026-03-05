use hyprland::dispatch::{Dispatch, DispatchType};

/// Helper function to call a dispatch and convert any errors into an mlua::Error with a
/// descriptive message.
pub fn call_dispatch(dispatch_type: DispatchType<'_>, action: &str) -> mlua::Result<()> {
    Dispatch::call(dispatch_type)
        .map_err(|e| mlua::Error::external(format!("Failed to {}: {}", action, e)))
}
