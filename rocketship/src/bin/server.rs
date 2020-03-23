#![feature(proc_macro_hygiene, decl_macro)]
extern crate chrono;
#[macro_use]
extern crate diesel;
extern crate dotenv;
#[macro_use]
extern crate rocket;
extern crate serde;
extern crate serde_derive;

pub mod lib;
pub mod models;
pub mod measurement;
pub mod schema;
pub mod handlers;

use diesel::prelude::*;
use rocket::Request;
use rocket_contrib::templates::Template;

use lib::*;
use models::*;

#[catch(404)]
fn not_found(_req: &Request) -> Template {
    let context = Context {title: "404".to_string(), lang: "en".to_string()};
    Template::render("404", &context)
}

fn main() {
    rocket::ignite()
        .register(catchers![not_found])
        .mount("/", routes![handlers::index,
                            handlers::handle_send_data,
                            handlers::api_get_measurements,
                            handlers::api_get_measurement,
                            handlers::api_get_sensors,
                            handlers::api_get_sensor,
                            handlers::api_get_labs,
                            handlers::api_get_lab,
                            handlers::api_get_phenomena,
                            handlers::api_get_phenomenon
        ])
        .attach(Template::fairing())
        .launch();
}
