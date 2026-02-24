use clap::Parser;

/// A wayland toolkit for building custom widgets using Lua
#[derive(Parser)]
#[command(version, long_about = None)]
pub struct Cli {
    /// The path to the Lua configuration file
    #[arg(short, long)]
    pub config: String,

    /// Set the logging level (error, warn, info, debug, trace).
    #[arg(short, long, default_value = "info")]
    pub log_level: String,
}
