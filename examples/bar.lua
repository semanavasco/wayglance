-- Example status bar using wayglance

-- helpers ------------------------------------------------------------------

local function shell(cmd)
  local handle = io.popen(cmd .. " 2>/dev/null")
  if not handle then
    return ""
  end
  local out = handle:read("*a")
  handle:close()
  return out:gsub("%s+$", "")
end

local function active_workspace()
  local out = shell("hyprctl activeworkspace -j")
  return tonumber(out:match('"id":%s*(%d+)')) or 1
end

local function window_title()
  local out = shell("hyprctl activewindow -j")
  return out:match('"title":%s*"([^"]*)"') or ""
end

-- state ---------------------------------------------------------------------

local WORKSPACES = 5
local ActiveWS = active_workspace()
local WinTitle = window_title()
local Clock = os.date("%H:%M")
local Date = os.date("%a %d %b")

-- widgets -------------------------------------------------------------------

local function workspace_button(id)
  return Button(
    Label(tostring(id), {
      class_list = { id == ActiveWS and "ws-active" or "ws-inactive" },
      valign = "center",
    }),
    {
      class_list = { "ws-btn" },
      valign = "center",
      on_click = function()
        shell("hyprctl dispatch workspace " .. id)
      end,
    }
  )
end

local function workspaces_widget()
  local btns = {}
  for i = 1, WORKSPACES do
    btns[i] = workspace_button(i)
  end
  return Container({
    id = "workspaces",
    orientation = "horizontal",
    spacing = 4,
    valign = "center",
    children = btns,
  })
end

local function title_widget()
  return Label(WinTitle, {
    id = "window-title",
    valign = "center",
  })
end

local function clock_widget()
  return Label(Clock, { id = "clock", valign = "center" })
end

local function date_widget()
  return Label(Date, { id = "date", valign = "center" })
end

local function spacer()
  return Container({ orientation = "horizontal", hexpand = true })
end

-- bar layout ----------------------------------------------------------------
-- Layout: [workspaces | title | -> spacer <- | clock | date]

return function()
  ActiveWS = active_workspace()
  WinTitle = window_title()
  Clock = os.date("%H:%M")
  Date = os.date("%a %d %b")

  return {
    title = "Bar",
    style = "bar.css",
    layer = "top",
    exclusive_zone = true,
    anchors = { top = true, left = true, right = true },

    child = Container({
      id = "bar",
      orientation = "horizontal",
      spacing = 8,
      valign = "center",
      children = {
        workspaces_widget(),
        title_widget(),
        spacer(),
        clock_widget(),
        date_widget(),
      },
    }),
  }
end
