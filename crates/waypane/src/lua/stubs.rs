use std::{borrow::Cow, fmt};

/// Trait for types that can generate Lua stubs for themselves.
pub trait Stubbed {
    fn stubs() -> Class;
}

/// Trait for types that can provide their Lua type name for documentation purposes.
///
/// This is used to generate accurate type annotations in the Lua stubs, especially for
/// objects like `Box<dyn Widget>`, `MaybeDynamic<T>`, `Option<T>`, etc...
pub trait LuaType {
    fn lua_type() -> Cow<'static, str>;
}

/// Represents a Lua module, including its name, optional parent module path, and documentation.
pub struct Module {
    pub name: &'static str,
    pub parent: Option<&'static str>,
    pub doc: &'static str,
}

impl Module {
    pub fn format_with_path(&self, full_path: &str) -> String {
        let mut out = String::new();
        if !self.doc.is_empty() {
            for line in self.doc.lines() {
                out.push_str(&format!("--- {}\n", line));
            }
        }
        out.push_str(&format!("{} = {{}}", full_path));
        out
    }
}

/// Represents an attribute of a Lua class, including its name, type, and documentation.
#[derive(Clone)]
pub struct Attr {
    pub name: &'static str,
    pub doc: &'static str,
    pub ty: Cow<'static, str>,
}

impl fmt::Display for Attr {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "---@field {} {}", self.name, self.ty)?;
        if !self.doc.is_empty() {
            write!(f, " {}", self.doc.replace('\n', " "))?;
        }
        Ok(())
    }
}

/// Represents a Lua class, including its name, parent classes, documentation, attributes, and
/// functions.
pub struct Class {
    pub name: &'static str,
    pub parents: Cow<'static, [Cow<'static, str>]>,
    pub doc: &'static str,
    pub attrs: Cow<'static, [Attr]>,
}

impl fmt::Display for Class {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if !self.doc.is_empty() {
            for line in self.doc.lines() {
                writeln!(f, "--- {}", line)?;
            }
        }

        let parents = if self.parents.is_empty() {
            "".to_string()
        } else {
            format!(" : {}", self.parents.join(", "))
        };

        writeln!(f, "---@class {}{}", self.name, parents)?;

        for attr in self.attrs.iter() {
            writeln!(f, "{}", attr)?;
        }

        writeln!(f, "local {} = {{}}", self.name)?;

        Ok(())
    }
}

/// Represents a Lua enum type alias (e.g. `---@alias Orientation "horizontal" | "vertical"`).
pub struct Enum {
    pub name: &'static str,
    pub doc: &'static str,
    pub variants: &'static str,
}

impl fmt::Display for Enum {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if !self.doc.is_empty() {
            for line in self.doc.lines() {
                writeln!(f, "--- {}", line)?;
            }
        }
        write!(f, "---@alias {} {}", self.name, self.variants)
    }
}

/// Represents the type of a Lua function, which can be either a standalone function or a method
/// of a class.
pub enum FnType {
    Function { module: Option<&'static str> },
    Method { class: &'static str },
}

/// Represents a Lua function, including its name, documentation, arguments, and return type.
pub struct Function {
    pub ty: FnType,
    pub name: &'static str,
    pub doc: &'static str,
    pub args: Cow<'static, [Attr]>,
    pub ret: Cow<'static, str>,
    pub ret_doc: &'static str,
}

impl fmt::Display for Function {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if !self.doc.is_empty() {
            for line in self.doc.lines() {
                writeln!(f, "--- {}", line)?;
            }
        }

        for arg in self.args.iter() {
            write!(f, "---@param {} {}", arg.name, arg.ty)?;
            if !arg.doc.is_empty() {
                write!(f, " {}", arg.doc.replace('\n', " "))?;
            }
            writeln!(f)?;
        }

        if self.ret != "nil" {
            write!(f, "---@return {}", self.ret)?;
            if !self.ret_doc.is_empty() {
                write!(f, " {}", self.ret_doc.replace('\n', " "))?;
            }
            writeln!(f)?;
        }

        let full_name = match &self.ty {
            FnType::Function { module } => {
                if let Some(module) = module {
                    format!("{}.{}", module, self.name)
                } else {
                    self.name.to_string()
                }
            }
            FnType::Method { class } => format!("{}:{}", class, self.name),
        };

        write!(
            f,
            "function {}({}) end",
            full_name,
            self.args
                .iter()
                .map(|a| a.name)
                .collect::<Vec<_>>()
                .join(", ")
        )
    }
}

/// Represents a widget builder function that creates a widget table with the `type` field set.
pub struct WidgetBuilder {
    pub name: &'static str,
    pub type_name: &'static str,
    pub class_name: &'static str,
    pub doc: &'static str,
}

impl fmt::Display for WidgetBuilder {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if !self.doc.is_empty() {
            for line in self.doc.lines() {
                writeln!(f, "--- {}", line)?;
            }
        }
        writeln!(
            f,
            "---@param config {} The configuration table for the {} widget.",
            self.class_name, self.class_name
        )?;
        writeln!(f, "---@return Widget widget The constructed widget.")?;
        write!(f, "function {}(config) end", self.name)
    }
}

/// A stub entry that can represent any kind of Lua type definition.
pub enum Stub {
    Module(Module),
    Class(Class),
    Enum(Enum),
    Function(Function),
    WidgetBuilder(WidgetBuilder),
}

/// Factory to build a [`Stub`] at runtime.
///
/// Because `Stub` variants aren't `const`, we register factory closures instead and call
/// `build()` at stub-generation time to get the actual instance.
pub struct StubFactory {
    pub build: fn() -> Stub,
}
inventory::collect!(StubFactory);
