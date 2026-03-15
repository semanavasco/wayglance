use clap::Parser;

/// A wayland toolkit for building custom widgets using Lua
#[derive(Parser)]
#[command(version, long_about = None)]
pub struct Cli {
    #[command(subcommand)]
    pub command: Command,
}

#[derive(Parser)]
pub enum Command {
    /// Run the waypane widget with the specified Lua shell configuration file
    Run {
        /// The path to the Lua configuration file
        config: String,

        /// Set the logging level (error, warn, info, debug, trace)
        #[arg(short, long, default_value = "info")]
        log_level: String,
    },
    /// Generate Lua stubs for the built-in waypane API
    GenStubs,
}
