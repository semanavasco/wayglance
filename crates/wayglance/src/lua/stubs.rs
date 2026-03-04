use std::borrow::Cow;
use std::fmt;

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

        Ok(())
    }
}

/// This is used to register classes in the inventory instead of the `Class` itself, since the
/// `Class` isn't const so it can't be directly registered. We can register this struct because a
/// function pointer is known at compile time.
/// We call build() at runtime to get the actual `Class` instance.
pub struct ClassStubFactory {
    pub build: fn() -> Class,
}
inventory::collect!(ClassStubFactory);
