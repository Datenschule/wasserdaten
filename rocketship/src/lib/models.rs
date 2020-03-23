use super::schema::*;
use chrono::prelude::*;
use serde_derive::{Serialize, Deserialize};


pub struct WaterLab(pub String);

#[derive(Debug)]
pub enum WaterLabError {
    Missing,
    Empty,
}

pub struct SensorPin(pub i32);

#[derive(Debug)]
pub enum SensorPinError {
    Missing,
    NaN
}

#[derive(Identifiable, Queryable, Associations)]
#[derive(Serialize, Deserialize, Debug)]
pub struct Measurement {
    pub id: i32,
    pub created_at: NaiveDateTime,
    pub lab: String,
    pub sensor: String,
    pub pin: i32,
    pub software_version: String,
}

#[derive(Insertable)]
#[derive(Serialize, Deserialize, Debug)]
#[table_name="measurements"]
pub struct NewMeasurement {
    pub created_at: NaiveDateTime,
    pub lab: String,
    pub sensor: String,
    pub pin: i32,
    pub software_version: String,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct RawMeasurement {
    pub sensor: String,
    pub software_version: String,
    pub sensordatavalues: Vec<RawMeasurementPair>
}

#[derive(Serialize, Deserialize, Debug)]
pub struct RawMeasurementPair {
    pub value_type: String,
    pub value: f32,
}

#[derive(Serialize, Deserialize, Debug)]
pub struct PayloadMeasurement {
    pub timestamp: NaiveDateTime,
    pub lab: String,
    pub sensor: String,
    pub pin: i32,
    pub software_version: String,
    pub sensordatavalues: Vec<RawMeasurementPair>
}

#[derive(Identifiable, Queryable, Associations)]
#[derive(Serialize, Deserialize, Debug)]
#[belongs_to(Measurement, foreign_key = "measurement_id")]
pub struct MeasurementPair {
    pub id: i32,
    pub measurement_id: i32,
    pub value_type: String,
    pub value_value: f32,
}

#[derive(Insertable)]
#[derive(Serialize, Deserialize, Debug)]
#[table_name="measurement_pairs"]
pub struct NewMeasurementPair<'a> {
    pub measurement_id: i32,
    pub value_type: &'a str,
    pub value_value: f32,
}
