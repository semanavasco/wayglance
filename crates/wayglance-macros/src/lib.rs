use proc_macro::TokenStream;
use quote::quote;
use syn::{Attribute, Data, DeriveInput, Expr, Fields, Lit, LitStr, Meta, parse_macro_input};

#[proc_macro_derive(LuaClass, attributes(lua_class, lua_attr))]
pub fn derive_stubbed(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);

    let ident = &input.ident;
    let mut lua_name = ident.to_string();

    let mut parent_classes = Vec::new();

    for attr in &input.attrs {
        if attr.path().is_ident("lua_class") {
            let _ = attr.parse_nested_meta(|meta| {
                if meta.path.is_ident("name") {
                    let value = meta.value()?;
                    let s: LitStr = value.parse()?;
                    lua_name = s.value();
                }
                Ok(())
            });
        }
    }

    let (attrs_init, parent_merges) = if let Data::Struct(struct_data) = &input.data
        && let Fields::Named(fields) = &struct_data.fields
    {
        let mut attr_quotes = Vec::new();
        let mut merge_quotes = Vec::new();

        for field in &fields.named {
            let field_name = field.ident.as_ref().expect("Expected named fields");
            let mut field_doc = extract_doc(&field.attrs);
            let field_type = &field.ty;

            let mut is_parent = false;
            let mut custom_name = field_name.to_string();
            let mut default_val = None;

            for attr in &field.attrs {
                if attr.path().is_ident("lua_attr") {
                    let _ = attr.parse_nested_meta(|meta| {
                        if meta.path.is_ident("parent") {
                            is_parent = true;
                        }

                        if meta.path.is_ident("name") {
                            let value = meta.value()?;
                            let s: LitStr = value.parse()?;
                            custom_name = s.value();
                        }

                        if meta.path.is_ident("default") {
                            let expr: Expr = meta.value()?.parse()?;

                            if let Expr::Lit(expr_lit) = &expr
                                && let Lit::Str(s) = &expr_lit.lit
                            {
                                default_val = Some(s.value());
                            } else {
                                default_val = Some(quote!(#expr).to_string());
                            }
                        }
                        Ok(())
                    });
                }
            }

            if let Some(default) = default_val {
                if !field_doc.is_empty() {
                    field_doc.push(' ');
                }

                field_doc.push_str(&format!("(Default: {})", default));
            }

            if is_parent {
                merge_quotes.push(quote! {
                    let parent_attrs = <#field_type as crate::lua::stubs::Stubbed>::stubs().attrs;
                    attrs_vec.extend(parent_attrs.into_owned());
                });
                parent_classes.push(
                    quote! { <#field_type as crate::lua::stubs::Stubbed>::stubs().name.into() },
                );
            } else {
                attr_quotes.push(quote! {
                    crate::lua::stubs::Attr {
                        name: #custom_name,
                        doc: #field_doc,
                        ty: <#field_type as crate::lua::stubs::LuaType>::lua_type(),
                    }
                });
            }
        }

        (quote! {vec![#(#attr_quotes),*]}, quote! {#(#merge_quotes)*})
    } else {
        return syn::Error::new_spanned(
            ident,
            "LuaClass can only be derived on structs with named fields",
        )
        .to_compile_error()
        .into();
    };

    let struct_doc = extract_doc(&input.attrs);

    let expanded = quote! {
        impl crate::lua::stubs::Stubbed for #ident {
            fn stubs() -> crate::lua::stubs::Class {
                let mut attrs_vec = #attrs_init;
                #parent_merges

                crate::lua::stubs::Class {
                    name: #lua_name,
                    parents: std::borrow::Cow::Owned(vec![#(#parent_classes),*]),
                    doc: #struct_doc,
                    attrs: std::borrow::Cow::Owned(attrs_vec),
                }
            }
        }
    };

    TokenStream::from(expanded)
}

/// Extracts doc comments from a list of attributes and concatenates them into a single string.
fn extract_doc(attrs: &[Attribute]) -> String {
    let mut docs = Vec::new();

    for attr in attrs {
        if attr.path().is_ident("doc")
            && let Meta::NameValue(meta) = &attr.meta
            && let Expr::Lit(expr) = &meta.value
            && let Lit::Str(lit_str) = &expr.lit
        {
            docs.push(lit_str.value().trim().to_string());
        }
    }

    docs.join("\n")
}

#[proc_macro_derive(LuaEnum)]
pub fn derive_lua_enum(input: TokenStream) -> TokenStream {
    let input = parse_macro_input!(input as DeriveInput);
    let name = &input.ident;

    let mut variants = Vec::new();

    if let Data::Enum(enum_data) = &input.data {
        for variant in &enum_data.variants {
            let variant_name = variant.ident.to_string().to_lowercase();
            variants.push(format!("\"{}\"", variant_name));
        }
    } else {
        return syn::Error::new_spanned(name, "LuaEnum can only be derived on enums")
            .to_compile_error()
            .into();
    }

    let lua_type_string = variants.join(" | ");

    let expanded = quote! {
        impl crate::lua::stubs::LuaType for #name {
            fn lua_type() -> std::borrow::Cow<'static, str> {
                std::borrow::Cow::Borrowed(#lua_type_string)
            }
        }
    };

    TokenStream::from(expanded)
}
