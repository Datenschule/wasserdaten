extern crate rocketship;
extern crate diesel;

use self::rocketship::*;
use self::models::*;
use self::diesel::prelude::*;

fn main() {
    use rocketship::schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        //.filter(sensor.eq(6982932))
        .limit(5)
        .load::<Measurement>(&connection)
        .expect("Error loading measurements");

    println!("Displaying {} measurements", results.len());
    for measurement in results {
        println!("{}", measurement.created_at);
        println!("{}", measurement.sensor);

        let pair_results: Vec<MeasurementPair> = <MeasurementPair as BelongingToDsl<&Measurement>>::belonging_to(&measurement)
            .load::<MeasurementPair>(&connection)
            .expect("Error loading m pairs");

        for pair in pair_results {
            println!("{}: {}", pair.value_type, pair.value_value);
        }
    }
}
