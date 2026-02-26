wayglance = {}

wayglance.setInterval = function(fn, ms)
  return {
    __wayglance_dynamic = "interval",
    callback = fn,
    interval = ms,
  }
end
