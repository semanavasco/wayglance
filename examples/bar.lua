-- Example status bar using wayglance

-- state ---------------------------------------------------------------------

local ActiveWorkspace = 1
local ActiveWindowTitle = ""

local function update_active_workspace()
  local monitors = wayglance.hyprland.getMonitors() or {}
  for _, monitor in ipairs(monitors) do
    if monitor.focused then
      ActiveWorkspace = monitor.active_workspace.id
      return
    end
  end
end

local function load_initial_state()
  update_active_workspace()

  local window = wayglance.hyprland.getActiveWindow() or {}
  if window and window.title then
    ActiveWindowTitle = window.title
  end
end

load_initial_state()

-- widgets -------------------------------------------------------------------

local function workspace_button(id, is_active)
  return Button({
    child = Label({
      text = tostring(id),
      class_list = { is_active and "ws-active" or "ws-inactive" },
      valign = "center",
    }),
    class_list = { "ws-btn" },
    valign = "center",
    on_click = function()
      wayglance.hyprland.switchWorkspace(id)
    end,
  })
end

local function workspaces_widget()
  return Container({
    id = "workspaces",
    orientation = "horizontal",
    spacing = 4,
    valign = "center",
    children = wayglance.onSignal({
      "hyprland::workspace_changed",
      "hyprland::workspace_added",
      "hyprland::workspace_deleted",
      "hyprland::workspace_moved",
      "hyprland::workspace_renamed",
      "hyprland::active_monitor_changed",
    }, function()
      update_active_workspace()

      local workspaces = wayglance.hyprland.getWorkspaces() or {}
      table.sort(workspaces, function(a, b)
        return a.workspace.id < b.workspace.id
      end)

      local btns = {}
      for _, ws_info in ipairs(workspaces) do
        local id = ws_info.workspace.id
        if type(id) == "number" and id > 0 then
          table.insert(btns, workspace_button(id, id == ActiveWorkspace))
        end
      end
      return btns
    end),
  })
end

local function title_widget()
  return Label({
    text = wayglance.onSignal("hyprland::active_window", function(window)
      if window and window.title then
        ActiveWindowTitle = window.title
      end

      return ActiveWindowTitle
    end),
    id = "window-title",
    valign = "center",
  })
end

local function clock_widget()
  return Label({
    text = wayglance.setInterval(function()
      return os.date("%H:%M")
    end, 1000),
    id = "clock",
    valign = "center",
  })
end

local function date_widget()
  return Label({
    text = wayglance.setInterval(function()
      return os.date("%a %d %b")
    end, 60000),
    id = "date",
    valign = "center",
  })
end

local function spacer()
  return Container({ orientation = "horizontal", hexpand = true })
end

-- bar layout ----------------------------------------------------------------
-- Layout: [workspaces | title | -> spacer <- | clock | date]

return function()
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
