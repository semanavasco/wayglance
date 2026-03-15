# Container

A container widget that can hold multiple child widgets, arranged either horizontally or vertically.

## Properties

In addition to the [Common Properties](./common.md), `Container` has the following properties:

| Property      | Type                          | Default      | Description                                                                                                                                   |
| ------------- | ----------------------------- | ------------ | --------------------------------------------------------------------------------------------------------------------------------------------- |
| `orientation` | `"horizontal" \| "vertical"`  | **required** | The direction in which to arrange the children.                                                                                               |
| `spacing`     | `number`                      | `0`          | The spacing between child widgets, in pixels.                                                                                                 |
| `children`    | `Widget[] \| State<Widget[]>` |              | A list of widgets to include in the container. If a state object is provided, the children will rebuild automatically when the state changes. |

## Examples

### Horizontal Bar

```lua
local my_bar = Container({
  orientation = "horizontal",
  spacing = 10,
  children = {
    Label({ text = "Item 1" }),
    Label({ text = "Item 2" }),
    Label({ text = "Item 3" }),
  },
})
```

### Vertical List

```lua
local my_list = Container({
  orientation = "vertical",
  spacing = 5,
  children = {
    Button({ child = Label({ text = "Home" }) }),
    Button({ child = Label({ text = "Settings" }) }),
    Button({ child = Label({ text = "Exit" }) }),
  },
})
```

### Dynamic Children

You can use reactive state to create dynamic containers.

```lua
local items_state = waypane.state({ "A", "B" })

local my_container = Container({
  orientation = "horizontal",
  children = items_state:as(function(list)
    local labels = {}
    for _, item in ipairs(list) do
      table.insert(labels, Label({ text = item }))
    end
    return labels
  end),
})

-- Adding an item will update the container
items_state:set({ "A", "B", "C" })
```
