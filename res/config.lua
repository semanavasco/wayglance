wayglance = {}

wayglance.setInterval = function(fn, ms)
  return {
    __wayglance_dynamic = "interval",
    callback = fn,
    interval = ms,
  }
end

wayglance.onSignal = function(signal, fn)
  return {
    __wayglance_dynamic = "signal",
    callback = fn,
    signal = signal,
  }
end

wayglance.emitSignal = function(signal, data) end
