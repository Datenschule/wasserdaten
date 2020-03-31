extern crate diesel;

use self::diesel::prelude::*;
use std::env::args;

use rocketlib::*;

fn main() {
    use schema::measurements::dsl::*;
    use schema::measurement_pairs::dsl::{measurement_pairs, measurement_id};

    let target = args().nth(1).expect("Expected a target to match against");
    let target = target.parse::<i32>().expect("Needs a number!");
    //let pattern = format!("%{}%", target);

    let connection = establish_connection();
    let mdeleted = diesel::delete(measurements.filter(id.eq(target)))
        .execute(&connection)
        .expect("Error deleting measurement");

    let mpdeleted = diesel::delete(measurement_pairs.filter(measurement_id.eq(target)))
        .execute(&connection)
        .expect("Error deleting measurement pairs");

    println!("Deleted {} measurement", mdeleted);
    println!("Deleted {} measurement pairs", mpdeleted);
}
