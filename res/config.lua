wayglance = {}

wayglance.setInterval = function(fn, ms)
  return {
    __wayglance_gen = true,
    callback = fn,
    interval = ms,
  }
end
