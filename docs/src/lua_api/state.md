# State Management

Reactive state is at the heart of `wayglance`. It allows you to create data points that your widgets can "watch", automatically updating themselves whenever the data changes.

## Creating State

You can create a new state object using the `wayglance.state()` function. You can provide an initial value for the state.

```lua
local my_state = wayglance.state("initial value")
```

## Getting and Setting State

State objects have `get()` and `set()` methods:

- `my_state:get()`: Returns the current value of the state.
- `my_state:set(new_value)`: Updates the state with a new value and triggers updates for any bound widgets.

```lua
local count = wayglance.state(0)

print(count:get()) -- Output: 0

count:set(1)
print(count:get()) -- Output: 1
```

## Binding State to Widgets

Many widget properties (like a Label's `text` or a Container's `children`) can accept a state object instead of a static value. When the state changes, the widget property will automatically update.

```lua
local title_state = wayglance.state("My Widget")

local my_label = Label({
  text = title_state,
})

-- Later...
title_state:set("Updated Title") -- The label's text will automatically update to "Updated Title"
```

## Transforming State

You can create a "transformed" version of a state object using the `:as()` method. This allows you to derive a new value from an existing state without manually managing multiple state objects.

The `:as()` method takes a function that receives the current state value and returns the transformed value.

```lua
local count = wayglance.state(5)

-- Create a derived state that formats the count
local label_text = count:as(function(val)
  return "The count is: " .. tostring(val)
end)

local my_label = Label({
  text = label_text,
})

-- Later...
count:set(10) -- The label will automatically update to "The count is: 10"
```

## Dynamic Children

Containers can also use reactive state for their `children` property. This is a powerful way to create dynamic lists or grids of widgets that can grow or shrink based on external data.

```lua
local items = wayglance.state({ "A", "B", "C" })

local my_container = Container({
  children = items:as(function(list)
    local labels = {}
    for _, item in ipairs(list) do
      table.insert(labels, Label({ text = item }))
    end
    return labels
  end),
})

-- Adding an item to the list will automatically rebuild the container's children
local current = items:get()
table.insert(current, "D")
items:set(current)
```
