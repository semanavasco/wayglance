-- Example background clock widget using wayglance

local function clock_widget()
  return Label(
    wayglance.setInterval(function()
      return os.date("%H:%M:%S")
    end, 1000),
    {
      id = "clock",
      halign = "center",
    }
  )
end

local function date_widget()
  return Label(
    wayglance.setInterval(function()
      return os.date("%A, %B %d, %Y")
    end, 60000),
    {
      id = "date",
      halign = "center",
    }
  )
end

return function()
  return {
    title = "Clock",
    style = "clock.css",
    layer = "background",
    exclusive_zone = false,
    anchors = {
      top = true,
      left = true,
      right = true,
      bottom = true,
    },
    monitors = { "eDP-1" },
    child = Container({
      type = "container",
      orientation = "vertical",
      valign = "center",
      halign = "center",
      spacing = 10,
      children = {
        clock_widget(),
        date_widget(),
      },
    }),
  }
end
