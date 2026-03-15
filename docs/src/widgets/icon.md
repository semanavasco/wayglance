# Icon

A widget that displays a GTK icon.

## Properties

In addition to the [Common Properties](./common.md), `Icon` has the following properties:

| Property       | Type                        | Default      | Description                                                             |
| -------------- | --------------------------- | ------------ | ----------------------------------------------------------------------- |
| `name`         | `string \| State<string>`   | **required** | The name of the icon to display (e.g. `"audio-volume-high"`).           |
| `size`         | `number \| State<number>`   | `24`         | The size of the icon in pixels.                                         |
| `use_fallback` | `boolean \| State<boolean>` | `true`       | Whether to use a fallback icon if the specified icon name is not found. |

## Examples

### Basic Icon

```lua
local my_icon = Icon({
  name = "audio-volume-high",
  size = 32,
})
```

### Dynamic Icon Name

You can use reactive state to change the icon based on external data.

```lua
local volume_state = waypane.state(50)

local volume_icon = Icon({
  name = volume_state:as(function(val)
    if val == 0 then return "audio-volume-muted" end
    if val < 33 then return "audio-volume-low" end
    if val < 66 then return "audio-volume-medium" end
    return "audio-volume-high"
  end),
  size = 24,
})
```
