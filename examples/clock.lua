-- Example background clock widget using wayglance

-- State to track the current theme
local current_theme = "theme-purple"

-- Widgets
local function clock_widget()
  return Label(
    wayglance.setInterval(function()
      return os.date("%H:%M:%S")
    end, 1000),
    {
      id = "clock",
      halign = "center",
      class_list = wayglance.onSignal("theme_changed", function(new_theme)
        current_theme = new_theme or current_theme
        return { "clock-text", current_theme }
      end),
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
      class_list = wayglance.onSignal("theme_changed", function(new_theme)
        current_theme = new_theme or current_theme
        return { "date-text", current_theme }
      end),
    }
  )
end

-- A helper to generate our theme-switching buttons
local function theme_button(target_theme, color_class)
  return Button(Label(""), {
    class_list = { "theme-btn", color_class },
    on_click = function()
      wayglance.emitSignal("theme_changed", target_theme)
    end,
  })
end

local function theme_switcher_widget()
  return Container({
    orientation = "horizontal",
    halign = "center",
    spacing = 15,
    class_list = { "switcher-container" },
    children = {
      theme_button("theme-purple", "btn-purple"),
      theme_button("theme-green", "btn-green"),
      theme_button("theme-orange", "btn-orange"),
    },
  })
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
    child = Container({
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
  }
end
