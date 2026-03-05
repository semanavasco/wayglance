-- Example status bar using wayglance

-- state ---------------------------------------------------------------------

local WorkspaceIds = {}
local ActiveWorkspace = 1
local ActiveWindowTitle = ""

local function load_initial_state()
  local workspaces = wayglance.hyprland.getWorkspaces() or {}

  for _, workspace in ipairs(workspaces) do
    if type(workspace.id) == "number" and workspace.id > 0 then
      WorkspaceIds[#WorkspaceIds + 1] = workspace.id
    end
  end

  table.sort(WorkspaceIds)

  if #WorkspaceIds == 0 then
    WorkspaceIds = { 1, 2, 3, 4, 5 }
  end

  local window = wayglance.hyprland.getActiveWindow() or {}

  if window and window.workspace and type(window.workspace.id) == "number" then
    ActiveWorkspace = window.workspace.id
  else
    ActiveWorkspace = WorkspaceIds[1]
  end

  if window and window.title then
    ActiveWindowTitle = window.title
  end
end

load_initial_state()

-- widgets -------------------------------------------------------------------

local function workspace_button(id)
  return Button({
    child = Label({
      text = tostring(id),
      class_list = wayglance.onSignal("hyprland::workspace_changed", function(workspace)
        if workspace and workspace.id then
          ActiveWorkspace = workspace.id
        end

        return { id == ActiveWorkspace and "ws-active" or "ws-inactive" }
      end),
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
  local btns = {}
  for i, workspace_id in ipairs(WorkspaceIds) do
    btns[i] = workspace_button(workspace_id)
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
