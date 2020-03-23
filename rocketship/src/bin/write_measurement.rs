extern crate chrono;
extern crate diesel;
extern crate rocketship;

use self::lib::*;
use self::measurement::*;

use std::io::{stdin, Read};
use chrono::prelude::*;
use chrono::{NaiveDateTime};

fn main() {
    let connection = establish_connection();

    println!("What lab?");
    let mut lab = String::new();
    stdin().read_line(&mut lab).unwrap();
    let sensor = &lab[..(lab.len() - 1)];

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
                                        lab.to_string(),
                                        sensor.to_string(),
                                        pin,
                                        "0.0.test".to_string()) {

        let pair = create_measurement_pair(&connection, m.id, "test", 23.99);

        println!("\nSaved measurement with id {}", m.id);
        println!("\nSaved measurement pair with id {}", pair.id);
    }
}

/*
#[cfg(not(windows))]
const EOF: &'static str = "CTRL+D";

#[cfg(windows)]
const EOF: &'static str = "CTRL+Z";
*/
