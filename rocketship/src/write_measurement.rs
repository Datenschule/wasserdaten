extern crate chrono;
extern crate diesel;

use std::io::{stdin};
use chrono::prelude::*;
use chrono::{NaiveDateTime};

use rocketlib::*;
use rocketlib::measurement::*;

fn main() {
    let connection = establish_connection();

    println!("What lab?");
    let mut lab = String::new();
    stdin().read_line(&mut lab).unwrap();
    let lab = &lab[..(lab.len() - 1)];

    println!("What sensor?");
    let mut sensor = String::new();
    stdin().read_line(&mut sensor).unwrap();
    let sensor = &sensor[..(sensor.len() - 1)];

    println!("What pin?");
    let mut pin = String::new();
    stdin().read_line(&mut pin).unwrap();
    let pin = &pin[..(pin.len() - 1)];
    let pin = &pin.parse::<i32>().unwrap();


    let utc: DateTime<Utc> = Utc::now();
    let ndt: NaiveDateTime = utc.naive_utc();

    if let Some(m) = create_measurement(&connection,
                                        ndt,
                                        &lab.to_string(),
                                        &sensor.to_string(),
                                        *pin,
                                        &"0.0.test".to_string()) {

        let p = create_measurement_pair(&connection,
                                m.id,
                                &"test_type".to_string(),
                                4.2).expect("Error saving pair");

        println!("\nSaved measurement with id {}", m.id);
        println!("\nSaved measurement pair with id {}", p.id);
    } else {
        println!("Error saving measurement");
    }
}

/*
#[cfg(not(windows))]
const EOF: &'static str = "CTRL+D";

#[cfg(windows)]
const EOF: &'static str = "CTRL+Z";
*/
