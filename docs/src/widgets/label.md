# Label

A simple widget that displays a text label.

## Properties

In addition to the [Common Properties](./common.md), `Label` has the following properties:

| Property | Type                      | Default      | Description                                                                                               |
| -------- | ------------------------- | ------------ | --------------------------------------------------------------------------------------------------------- |
| `text`   | `string \| State<string>` | **required** | The text content of the label. Can be a static string or a dynamic expression that evaluates to a string. |

## Examples

### Static Text

```lua
local my_label = Label({
  text = "Hello, world!",
  halign = "center",
})
```

### Dynamic Text

```lua
local time_state = wayglance.state(os.date("%H:%M"))

local clock_label = Label({
  text = time_state,
})

-- Update the state every minute
wayglance.setInterval(function()
  time_state:set(os.date("%H:%M"))
end, 60000)
```
