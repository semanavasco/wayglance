local function make_builder(type_name, primary_field)
  return function(first, second)
    local config
    if type(first) == "table" and (not primary_field or first.type == nil) then
      config = first
    else
      config = second or {}
      if primary_field then
        config[primary_field] = first
      end
    end
    config.type = type_name:lower()
    return config
  end
end

Label = make_builder("Label", "text")

Container = make_builder("Container", nil)

Button = make_builder("Button", "child")
