# Timers

`wayglance` provides a set of timer functions that allow you to execute Lua code after a delay or at regular intervals. These timers are integrated with the GTK main loop, ensuring they don't block the UI thread.

## `wayglance.setTimeout(callback, milliseconds)`

Executes a callback function once after a specified delay.

- **callback**: The function to execute.
- **milliseconds**: The delay in milliseconds.
- **Returns**: A `CancelHandle`.

**Example:**

```lua
local handle = wayglance.setTimeout(function()
  print("This message appears after 2 seconds!")
end, 2000)

-- You can cancel it before it fires:
handle:cancel()
```

## `wayglance.setInterval(callback, milliseconds)`

Executes a callback function repeatedly at a specified interval.

- **callback**: The function to execute.
- **milliseconds**: The interval in milliseconds.
- **Returns**: A `CancelHandle`.

**Example:**

```lua
local handle = wayglance.setInterval(function()
  print("This message appears every second!")
end, 1000)

-- Stop the interval later:
handle:cancel()
```

## CancelHandle

The `CancelHandle` returned by timer functions has a single method:

- **`:cancel()`**: Stops the timer or interval.

  _If called on a `setTimeout` that has already fired or an interval that has already been cancelled, it does nothing._

## Example: Periodic Update

Timers are commonly used to update state values periodically, such as for a clock widget.

```lua
local time_state = wayglance.state(os.date("%H:%M:%S"))

local timer = wayglance.setInterval(function()
  time_state:set(os.date("%H:%M:%S"))
end, 1000)

-- If you ever need to stop the clock:
-- timer:cancel()

local clock_label = Label({
  text = time_state,
})
```
