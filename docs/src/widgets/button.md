# Button

A clickable button widget.

## Properties

In addition to the [Common Properties](./common.md), `Button` has the following properties:

| Property   | Type         | Default      | Description                                     |
| ---------- | ------------ | ------------ | ----------------------------------------------- |
| `child`    | `Widget`     | **required** | The child widget to display inside the button.  |
| `on_click` | `function()` | **required** | Function to execute when the button is clicked. |

## Examples

### Clickable Label

```lua
local my_button = Button({
  child = Label({ text = "Click Me" }),
  on_click = function()
    print("The button was clicked!")
  end,
})
```

### Clickable Icon

```lua
local my_button = Button({
  child = Icon({ name = "firefox", size = 32 }),
  on_click = function()
    os.execute("firefox &")
  end,
})
```

### Using Common Properties

You can apply common properties to customize the button's appearance and behavior.

```lua
local my_button = Button({
  id = "my-button",
  class_list = { "primary" },
  child = Label({ text = "Submit" }),
  on_click = function()
    print("Submitting...")
  end,
})
```
