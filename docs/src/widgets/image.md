# Image

A widget that displays an image from a file path.

## Properties

In addition to the [Common Properties](./common.md), `Image` has the following properties:

| Property            | Type                        | Default      | Description                                                                                |
| ------------------- | --------------------------- | ------------ | ------------------------------------------------------------------------------------------ |
| `src`               | `string \| State<string>`   | **required** | The file path to the image. Relative paths are resolved against the config file directory. |
| `alternative_text`  | `string \| State<string>`   |              | The alternative textual description for the picture.                                       |
| `keep_aspect_ratio` | `boolean \| State<boolean>` | `true`       | Whether to maintain the aspect ratio of the image.                                         |
| `can_shrink`        | `boolean \| State<boolean>` | `true`       | Whether the image can be shrunk to smaller than its original size.                         |

## Examples

### Local Image

```lua
local my_image = Image({
  src = "assets/logo.png",
  width_request = 100,
  height_request = 100,
})
```

### Absolute Path

```lua
local my_image = Image({
  src = "/home/user/Pictures/wallpaper.jpg",
  keep_aspect_ratio = true,
})
```

### Dynamic Image

You can change the image source based on reactive state.

```lua
local profile_pic_state = wayglance.state("assets/default.png")

local user_avatar = Image({
  src = profile_pic_state,
  width_request = 48,
  height_request = 48,
})

-- Later...
profile_pic_state:set("assets/user-1.png")
```
