extern crate diesel;

use self::diesel::prelude::*;
use rocketlib::*;
use rocketlib::models::*;

fn main() {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        //.filter(sensor.eq(6982932))
        //.limit(5)
        .load::<Measurement>(&connection)
        .expect("Error loading measurements");

    println!("Displaying {} measurements", results.len());
    for measurement in results {
        println!("\n#{} -- {}", measurement.id, measurement.created_at);
        println!("sensor {}", measurement.sensor);

        let pair_results: Vec<MeasurementPair> = <MeasurementPair as BelongingToDsl<&Measurement>>::belonging_to(&measurement)
            .load::<MeasurementPair>(&connection)
            .expect("Error loading m pairs");

        for pair in pair_results {
            println!("{}: {}", pair.value_type, pair.value_value);
        }
    }
}
