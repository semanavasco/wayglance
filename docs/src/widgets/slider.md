# Slider

A widget that allows users to select a value from a range by sliding a handle.

## Properties

In addition to the [Common Properties](./common.md), `Slider` has the following properties:

| Property      | Type                                          | Default        | Description                                                                                   |
| ------------- | --------------------------------------------- | -------------- | --------------------------------------------------------------------------------------------- |
| `value`       | `number \| State<number>`                     | `0.0`          | The current value of the slider.                                                              |
| `min`         | `number \| State<number>`                     | `0.0`          | The minimum value of the slider.                                                              |
| `max`         | `number \| State<number>`                     | `100.0`        | The maximum value of the slider.                                                              |
| `step`        | `number \| State<number>`                     | `1.0`          | The step increment for the slider.                                                            |
| `page_step`   | `number \| State<number>`                     | `10.0`         | The page step increment for the slider.                                                       |
| `digits`      | `number \| State<number>`                     | `1`            | The number of decimal digits for the slider value.                                            |
| `draw_value`  | `boolean \| State<boolean>`                   | `true`         | Whether to draw the current value next to the slider.                                         |
| `orientation` | `"horizontal" \| "vertical" \| State<string>` | `"horizontal"` | The orientation of the slider.                                                                |
| `on_change`   | `function(value)`                             | **required**   | A callback that triggers when the slider's value changes. Receives the new value as argument. |

## Examples

### Volume Slider

```lua
local volume_state = waypane.state(50)

local volume_slider = Slider({
  min = 0,
  max = 100,
  value = volume_state,
  on_change = function(val)
    volume_state:set(val)
    os.execute(string.format("pamixer --set-volume %.0f", val))
  end,
})
```

### Vertical Brightness Slider

```lua
local brightness_slider = Slider({
  orientation = "vertical",
  min = 0,
  max = 100,
  value = 50,
  draw_value = false,
  on_change = function(val)
    os.execute(string.format("brightnessctl s %.0f%%", val))
  end,
})
```
