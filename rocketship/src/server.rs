#![feature(proc_macro_hygiene, decl_macro)]
#[macro_use]
extern crate rocket;

//use diesel::prelude::*;
use rocket_contrib::templates::Template;
use rocketlib::*;

fn main() {
    rocket::ignite()
        .register(catchers![rocketlib::catchers::not_found])
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
