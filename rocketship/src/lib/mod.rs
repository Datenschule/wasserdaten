#![feature(proc_macro_hygiene, decl_macro)]
extern crate chrono;
#[macro_use]
extern crate diesel;
extern crate dotenv;
#[macro_use]
extern crate rocket;
extern crate serde;
extern crate serde_derive;

use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use serde_derive::{Serialize};
use std::env;

pub mod models;
pub mod measurement;
pub mod schema;
pub mod handlers;
pub mod catchers;

#[derive(Serialize)]
pub struct Context {
    pub title: String,
    pub lang: String
}

pub fn establish_connection() -> PgConnection {
    dotenv().ok();

    let database_url = env::var("DATABASE_URL")
        .expect("DATABASE_URL must be set");
    PgConnection::establish(&database_url)
        .expect(&format!("Error connecting to {}", database_url))
}
