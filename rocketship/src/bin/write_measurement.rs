extern crate chrono;
extern crate diesel;
extern crate rocketship;

use chrono::prelude::*;
use self::rocketship::*;
use std::io::{stdin, Read};

fn main() {
    let connection = establish_connection();

    println!("What sensor?");
    let mut sensor = String::new();
    stdin().read_line(&mut sensor).unwrap();
    let sensor = &sensor[..(sensor.len() - 1)]; // Drop the newline character
    let sensor = &sensor.parse::<i32>().unwrap();

    println!("What sampling rate?");
    let mut sample = String::new();
    stdin().read_line(&mut sample).unwrap();
    let sample = &sample[..(sample.len() - 1)]; // Drop the newline character
    let sample = sample.parse::<i32>().unwrap();

    let utc: DateTime<Utc> = Utc::now();
    let time: NaiveDateTime = utc.naive_utc();

    let measurement = create_measurement(&connection, time, sample, *sensor);

    let pair = create_measurement_pair(&connection, measurement.id, "test", 23.99);
    println!("\nSaved measurement with id {}", measurement.id);
    println!("\nSaved measurement pair with id {}", pair.id);
}

#[cfg(not(windows))]
const EOF: &'static str = "CTRL+D";

#[cfg(windows)]
const EOF: &'static str = "CTRL+Z";
