# Shell & Windows

The `Shell` and `Window` objects are the top-level structures used to define your `wayglance` configuration.

## The Shell

The `Shell` object is created using `wayglance.shell()`. It manages global settings and acts as a container for your windows.

### Properties

| Property | Type     | Default       | Description                                            |
| -------- | -------- | ------------- | ------------------------------------------------------ |
| `title`  | `string` | `"wayglance"` | The application title, used as the GTK application ID. |
| `style`  | `string` |               | An optional path to a global CSS stylesheet.           |

```lua
local shell = wayglance.shell({
  title = "My Bar",
  style = "style.css",
})
```

## Windows

A `Window` is defined using the `shell:window(name, config)` method. Each window represents a standalone UI element on the screen.

### Window Configuration Properties

| Property         | Type                                             | Default      | Description                                                                                                                |
| ---------------- | ------------------------------------------------ | ------------ | -------------------------------------------------------------------------------------------------------------------------- |
| `monitors`       | `string[]`                                       |              | A list of monitor names (e.g., `{"eDP-1", "DP-1"}`). If omitted, the window appears on all monitors.                       |
| `layer`          | `"background" \| "bottom" \| "top" \| "overlay"` | **required** | The layer where the window should be placed.                                                                               |
| `exclusive_zone` | `boolean`                                        | `false`      | Whether the window should reserve space on the screen (e.g., a status bar that shifts other windows).                      |
| `anchors`        | [`Anchors`](#anchors)                            |              | A table specifying which edges of the screen the window should be anchored to.                                             |
| `margins`        | [`Margins`](#margins)                            |              | Margins around the window when anchored.                                                                                   |
| `layout`         | `function(monitor) \| Widget`                    | **required** | A function that returns a widget tree, or a single static widget. The function receives a `monitor` object as an argument. |

### Monitor Information

When `layout` is a function, it receives a `monitor` table with the following fields:

| Field          | Type     | Description                            |
| -------------- | -------- | -------------------------------------- |
| `name`         | `string` | The connector name (e.g., "eDP-1").    |
| `id`           | `number` | The unique identifier for the monitor. |
| `width`        | `number` | Monitor width in pixels.               |
| `height`       | `number` | Monitor height in pixels.              |
| `scale`        | `number` | UI scale factor.                       |
| `refresh_rate` | `number` | Refresh rate in Hz.                    |

### Anchors

A table specifying which edges of the screen the window should be anchored to. All fields default to `false`.

| Field    | Type      | Default | Description                              |
| -------- | --------- | ------- | ---------------------------------------- |
| `top`    | `boolean` | `false` | Anchor to the top edge of the screen.    |
| `bottom` | `boolean` | `false` | Anchor to the bottom edge of the screen. |
| `left`   | `boolean` | `false` | Anchor to the left edge of the screen.   |
| `right`  | `boolean` | `false` | Anchor to the right edge of the screen.  |

### Margins

A table specifying pixel margins from each edge. All fields default to `0`. This type is also used by the [Common Widget Properties](../widgets/common.md).

| Field    | Type     | Default | Description                       |
| -------- | -------- | ------- | --------------------------------- |
| `top`    | `number` | `0`     | Margin from the top in pixels.    |
| `bottom` | `number` | `0`     | Margin from the bottom in pixels. |
| `left`   | `number` | `0`     | Margin from the left in pixels.   |
| `right`  | `number` | `0`     | Margin from the right in pixels.  |

## Window Visibility

The visibility of a window is automatically managed based on its root widget. If the root widget's `visible` property is set to `false`, the window will be hidden from the compositor. This is particularly useful for creating togglable UI elements.

### Example: Togglable Dashboard

```lua
local is_visible = wayglance.state(false)

shell:window("dashboard", {
  layer = "overlay",
  anchors = { top = true, bottom = true, left = true, right = true },
  layout = Container({
    visible = is_visible,
    children = {
      Label({ text = "Dashboard Content" })
    }
  })
})

-- Toggle visibility elsewhere
wayglance.onSignal("toggle_dashboard", function()
  is_visible:set(not is_visible:get())
end)
```

### Complete Configuration Example

A typical `config.lua` returns the shell object:

```lua
local shell = wayglance.shell({ title = "My Desktop" })

shell:window("bar", {
  layer = "top",
  exclusive_zone = true,
  anchors = { top = true, left = true, right = true },
  layout = Label({ text = "Hello!" })
})

return shell
```
