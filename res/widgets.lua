local function make_builder(type_name, primary_field)
  return function(first, second)
    local config
    -- If first is a table, it could be the config OR a value (if primary_field is set)
    -- We consider it a full config if it doesn't have a type (not a widget)
    -- and isn't wayglance-generated (which would have __wayglance_dynamic set)
    if type(first) == "table" and (not primary_field or (first.type == nil and not first.__wayglance_dynamic)) then
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
