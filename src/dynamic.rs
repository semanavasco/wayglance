//! This module provides the reactivity system for wayglance.
//!
//! It allows widget properties to be either static values or dynamic values that update based on
//! timers (intervals) or event-driven signals.

use std::cell::{Cell, RefCell};
use std::collections::HashMap;
use std::rc::Rc;
use std::{any::type_name, time::Duration};

use anyhow::{Context, Result};
use gtk4::{
    glib::{self, object::IsA},
    prelude::WidgetExt,
};
use mlua::FromLua;

use crate::shell::config::LUA;

/// A value that is either resolved statically at parse time, or computed dynamically.
pub enum MaybeDynamic<T> {
    /// A plain value of type `T`.
    Static(T),
    /// A dynamic value computed by a Lua callback every `interval` milliseconds.
    Interval {
        callback: mlua::RegistryKey,
        interval: u64,
    },
    /// A dynamic value computed by a Lua callback whenever any of the specified signals are
    /// emitted.
    Signal {
        callback: mlua::RegistryKey,
        signals: Vec<String>,
    },
}

impl<T> FromLua for MaybeDynamic<T>
where
    T: FromLua,
{
    fn from_lua(value: mlua::Value, lua: &mlua::Lua) -> mlua::Result<Self> {
        if let mlua::Value::Table(ref t) = value
            && let Ok(wayglance_d) = t.get::<String>("__wayglance_dynamic")
        {
            match wayglance_d.as_str() {
                "interval" => {
                    let callback: mlua::Function = t.get("callback")?;
                    let registry = lua.create_registry_value(callback)?;
                    let interval = t.get("interval")?;

                    return Ok(MaybeDynamic::Interval {
                        callback: registry,
                        interval,
                    });
                }
                "signal" => {
                    let callback: mlua::Function = t.get("callback")?;
                    let registry = lua.create_registry_value(callback)?;
                    let signals_val: mlua::Value = t.get("signal")?;

                    let signals = match signals_val {
                        mlua::Value::String(s) => vec![s.to_str()?.to_string()],
                        mlua::Value::Table(t) => t
                            .sequence_values::<String>()
                            .collect::<mlua::Result<Vec<String>>>()?,
                        _ => {
                            return Err(mlua::Error::FromLuaConversionError {
                                from: signals_val.type_name(),
                                to: "String or Table of Strings".to_string(),
                                message: Some(
                                    "Expected a signal name or a list of signal names".to_string(),
                                ),
                            });
                        }
                    };

                    return Ok(MaybeDynamic::Signal {
                        callback: registry,
                        signals,
                    });
                }
                _ => {
                    return Err(mlua::Error::FromLuaConversionError {
                        from: "string",
                        to: type_name::<MaybeDynamic<T>>().to_string(),
                        message: Some(
                            "Invalid dynamic value type (expected 'signal' or 'interval')"
                                .to_string(),
                        ),
                    });
                }
            }
        }

        let val_type = value.type_name();
        T::from_lua(value, lua)
            .map(MaybeDynamic::Static)
            .map_err(|_| mlua::Error::FromLuaConversionError {
                from: val_type,
                to: type_name::<MaybeDynamic<T>>().to_string(),
                message: Some(
                    "Expected a wayglance dynamic value (e.g. wayglance.setInterval(...))"
                        .to_string(),
                ),
            })
    }
}

impl<T> MaybeDynamic<T>
where
    T: FromLua + Clone,
{
    /// Binds this value to a GTK widget property.
    ///
    /// - If `Static`: The value is applied immediately.
    /// - If `Interval`: A GLib timeout is set up to update the property periodically.
    /// - If `Signal`: The property updates whenever one of the signals is emitted.
    ///
    /// All dynamic bindings are automatically cleaned up when the widget is destroyed.
    pub fn bind<W, F>(&self, widget: &W, prop_name: &'static str, mut apply_fn: F) -> Result<()>
    where
        W: IsA<gtk4::Widget>,
        F: FnMut(&W, T) + 'static,
    {
        match self {
            MaybeDynamic::Static(val) => {
                apply_fn(widget, val.clone());
                Ok(())
            }
            MaybeDynamic::Interval { callback, interval } => {
                bind_interval(widget, callback, *interval, prop_name, apply_fn)
            }
            MaybeDynamic::Signal { callback, signals } => {
                bind_signals(widget, callback, signals, prop_name, apply_fn)
            }
        }
    }
}

/// Internal helper to bind an interval-based dynamic value.
fn bind_interval<T, W, F>(
    widget: &W,
    callback_key: &mlua::RegistryKey,
    interval: u64,
    prop_name: &'static str,
    mut apply_fn: F,
) -> Result<()>
where
    T: FromLua,
    W: IsA<gtk4::Widget>,
    F: FnMut(&W, T) + 'static,
{
    let lua = LUA.get().context("Lua instance not initialized")?;
    let callback = lua.registry_value::<mlua::Function>(callback_key)?;
    let widget_clone = widget.clone();

    // Initial call to set the value immediately
    callback
        .call::<T>(())
        .map(|val| apply_fn(&widget_clone, val))?;

    // Set up a GLib timeout to update the value periodically
    let source_id = glib::timeout_add_local(Duration::from_millis(interval), move || {
        match callback.call::<T>(()) {
            Ok(val) => apply_fn(&widget_clone, val),
            Err(e) => tracing::error!("Error calling Lua callback for {}: {}", prop_name, e),
        }
        glib::ControlFlow::Continue
    });

    let source_id = Cell::new(Some(source_id));
    widget.connect_destroy(move |_| {
        if let Some(id) = source_id.take() {
            id.remove();
        }
    });
    Ok(())
}

thread_local! {
    /// Thread-local bus for broadcasting signals across the application.
    pub static SIGNAL_BUS: RefCell<SignalBus> = RefCell::new(SignalBus::default());
}

type SignalCallback = Box<dyn Fn(mlua::Value)>;

/// Simple publish-subscribe bus for named signals.
#[derive(Default)]
pub struct SignalBus {
    listeners: HashMap<String, HashMap<usize, SignalCallback>>,
    next_id: usize,
}

impl SignalBus {
    /// Subscribes a callback to a signal name. Returns a unique subscription ID.
    pub fn subscribe(&mut self, signal: &str, cb: SignalCallback) -> usize {
        let id = self.next_id;
        self.next_id += 1;
        self.listeners
            .entry(signal.to_string())
            .or_default()
            .insert(id, cb);
        id
    }

    /// Unsubscribes a listener using its signal name and subscription ID.
    pub fn unsubscribe(&mut self, signal: &str, id: usize) {
        if let Some(callbacks) = self.listeners.get_mut(signal) {
            callbacks.remove(&id);
        }
    }

    /// Emits a signal to all subscribed listeners, passing along optional data.
    pub fn emit(&self, signal: &str, data: mlua::Value) {
        if let Some(callbacks) = self.listeners.get(signal) {
            for cb in callbacks.values() {
                cb(data.clone());
            }
        }
    }
}

/// Internal helper to bind a signal-based dynamic value.
fn bind_signals<T, W, F>(
    widget: &W,
    callback_key: &mlua::RegistryKey,
    signals: &[String],
    prop_name: &'static str,
    mut apply_fn: F,
) -> Result<()>
where
    T: FromLua,
    W: IsA<gtk4::Widget>,
    F: FnMut(&W, T) + 'static,
{
    let lua = LUA.get().context("Lua instance not initialized")?;
    let callback = lua.registry_value::<mlua::Function>(callback_key)?;
    let widget_clone = widget.clone();

    // Initial call to set the value immediately
    match callback.call::<T>(()) {
        Ok(val) => apply_fn(&widget_clone, val),
        Err(e) => tracing::error!("Error calling Lua callback for {}: {}", prop_name, e),
    }

    // Use Rc to share the callback and apply_fn across multiple signal listeners
    let apply_fn_cell = Rc::new(RefCell::new(apply_fn));
    let callback_rc = Rc::new(callback);

    let mut subscription_ids = Vec::new();

    for signal in signals {
        let widget_clone = widget.clone();
        let apply_fn_cell = Rc::clone(&apply_fn_cell);
        let callback_rc = Rc::clone(&callback_rc);

        let listener = Box::new(move |data: mlua::Value| match callback_rc.call::<T>(data) {
            Ok(val) => apply_fn_cell.borrow_mut()(&widget_clone, val),
            Err(e) => tracing::error!("Error calling Lua callback for {}: {}", prop_name, e),
        });

        let subscribe_id = SIGNAL_BUS.with(|bus| bus.borrow_mut().subscribe(signal, listener));
        subscription_ids.push((signal.clone(), subscribe_id));
    }

    widget.connect_destroy(move |_| {
        SIGNAL_BUS.with(|bus| {
            let mut bus = bus.borrow_mut();
            for (signal, id) in &subscription_ids {
                bus.unsubscribe(signal, *id);
            }
        });
    });

    Ok(())
}
