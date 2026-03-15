# ProgressBar

A widget that displays a progress bar.

## Properties

In addition to the [Common Properties](./common.md), `ProgressBar` has the following properties:

| Property      | Type                                          | Default        | Description                                                    |
| ------------- | --------------------------------------------- | -------------- | -------------------------------------------------------------- |
| `fraction`    | `number \| State<number>`                     | `0.0`          | The fraction of the progress bar that is filled (`0.0`–`1.0`). |
| `text`        | `string \| State<string>`                     |                | The text to display over the progress bar if provided.         |
| `inverted`    | `boolean \| State<boolean>`                   | `false`        | Whether the progress bar is inverted.                          |
| `orientation` | `"horizontal" \| "vertical" \| State<string>` | `"horizontal"` | The orientation of the progress bar.                           |

## Examples

### Static Progress

```lua
local my_bar = ProgressBar({
  fraction = 0.5,
  text = "50%",
})
```

### Dynamic Progress

You can use reactive state to update the progress bar based on external data.

```lua
local cpu_state = waypane.state(0.1)

local cpu_bar = ProgressBar({
  fraction = cpu_state,
  text = cpu_state:as(function(val)
    return string.format("CPU: %.0f%%", val * 100)
  end),
})

-- Update the CPU state periodically...
```

### Vertical Inverted Bar

```lua
local volume_bar = ProgressBar({
  orientation = "vertical",
  inverted = true,
  fraction = 0.7,
})
```
