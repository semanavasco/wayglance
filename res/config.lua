wayglance = {}

--- Creates a dynamic entry that updates every `ms` milliseconds by calling `fn`.
---@param fn fun(): any Function that returns the value to be displayed.
---@param ms number Interval in milliseconds for how often to update the entry using `fn`.
wayglance.setInterval = function(fn, ms)
  return {
    __wayglance_dynamic = "interval",
    callback = fn,
    interval = ms,
  }
end

--- Creates a dynamic entry that updates whenever a signal is emitted.
--- The entry will update when any of the specified signals are emitted.
---@param signal string|string[] Signal or list of signals to listen for.
---@param fn fun(data: any?): any Function that returns the value to be displayed. If the signal is emitted with data, it will be passed as an argument to this function.
wayglance.onSignal = function(signal, fn)
  return {
    __wayglance_dynamic = "signal",
    callback = fn,
    signal = signal,
  }
end

--- Emits a signal with optional data. Any entry created with `wayglance.onSignal`
--- that listens for this signal will update.
--- If `data` is provided, it will be passed to the callback function of the entries
--- that listen for this signal.
---
--- This function is implemented in Rust.
---@param signal string Name of the signal to emit. Entries created with `wayglance.onSignal` that listen for this signal will update.
---@param data any? Optional data to pass to the callback functions of entries that listen for this signal. If not provided, the callbacks will receive `nil`.
wayglance.emitSignal = function(signal, data) end
