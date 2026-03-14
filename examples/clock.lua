-- Example background clock widget using wayglance

-- State
local current_theme = wayglance.state("theme-purple")

-- Widgets
local function clock_widget()
  local time_state = wayglance.state(os.date("%H:%M:%S"))

  wayglance.setInterval(function()
    time_state:set(os.date("%H:%M:%S"))
  end, 1000)

  return Label({
    text = time_state,
    id = "clock",
    halign = "center",
    class_list = current_theme:as(function(theme)
      return { "clock-text", theme }
    end),
  })
end

local function date_widget()
  local date_state = wayglance.state(os.date("%A, %B %d, %Y"))

  wayglance.setInterval(function()
    date_state:set(os.date("%A, %B %d, %Y"))
  end, 60000)

  return Label({
    text = date_state,
    id = "date",
    halign = "center",
    class_list = current_theme:as(function(theme)
      return { "date-text", theme }
    end),
  })
end

-- A helper to generate our theme-switching buttons
local function theme_button(target_theme, color_class, hex_color, pretty_name)
  return Button({
    child = Label({ text = "" }),
    class_list = { "theme-btn", color_class },
    on_click = function()
      current_theme:set(target_theme)
    end,
    visible = current_theme:as(function(theme)
      return theme ~= target_theme
    end),
    tooltip = string.format(
      "<span font_weight='bold' foreground='%s'>Activate %s Theme</span>\n"
        .. "<span size='small'>Changes the global color scheme</span>",
      hex_color,
      pretty_name
    ),
  })
end

local function theme_switcher_widget()
  return Container({
    orientation = "horizontal",
    halign = "center",
    spacing = 15,
    class_list = { "switcher-container" },
    children = {
      theme_button("theme-purple", "btn-purple", "#cba6f7", "Purple"),
      theme_button("theme-green", "btn-green", "#a6e3a1", "Green"),
      theme_button("theme-orange", "btn-orange", "#fab387", "Orange"),
    },
  })
end

local shell = wayglance.shell({
  title = "Clock",
  style = "clock.css",
})

shell:window("clock-window", {
  layer = "background",
  exclusive_zone = false,
  anchors = {
    top = true,
    left = true,
    right = true,
    bottom = true,
  },
  layout = Container({
    orientation = "vertical",
    valign = "center",
    halign = "center",
    spacing = 10,
    children = {
      clock_widget(),
      date_widget(),
      theme_switcher_widget(),
    },
  }),
})

return shell
