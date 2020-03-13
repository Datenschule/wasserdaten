use super::*;
use chrono::prelude::*;
use diesel::pg::PgConnection;

pub fn create_measurement<'a>(conn: &PgConnection,
                              created_at: NaiveDateTime,
                              lab: &'a str,
                              sensor: &'a str,
                              pin: i32,
                              software_version: &'a str) -> Option<Measurement> {
    use schema::measurements;

    let new_measurement = NewMeasurement {
        created_at,
        lab: lab.to_string(),
        sensor: sensor.to_string(),
        pin,
        software_version: software_version.to_string()
    };

    let measurement = diesel::insert_into(measurements::table)
        .values(&new_measurement)
        .get_result(conn)
        .expect("Error saving new measurement");

    Some(measurement)
}

pub fn create_measurement_pair<'a>(conn: &PgConnection,
                                   measurement_id: i32,
                                   value_type: &'a str,
                                   value_value: f32) -> Option<MeasurementPair> {
    use schema::measurement_pairs;

    let new_measurement_pair = NewMeasurementPair {
        measurement_id,
        value_type,
        value_value
    };

    let measurement_pair = diesel::insert_into(measurement_pairs::table)
        .values(&new_measurement_pair)
        .get_result(conn)
        .expect("Error saving new measurement pair");

    Some(measurement_pair)
}
