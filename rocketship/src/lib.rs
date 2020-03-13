use diesel::prelude::*;
use diesel::pg::PgConnection;
use dotenv::dotenv;
use serde_derive::{Serialize};
use std::env;

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
