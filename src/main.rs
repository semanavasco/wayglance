mod cli;
mod dynamic;
mod shell;
mod widgets;

use anyhow::Result;
use clap::Parser;
use cli::Cli;
use gtk4::glib::ExitCode;

use crate::shell::{config::Config, run_app};

fn main() -> Result<ExitCode> {
    let cli = Cli::parse();

    let env_filter = tracing_subscriber::EnvFilter::try_from_default_env()
        .unwrap_or_else(|_| tracing_subscriber::EnvFilter::new(&cli.log_level));

    tracing_subscriber::fmt().with_env_filter(env_filter).init();

    let config = Config::load(&cli.config)?;

    run_app(config)
}
