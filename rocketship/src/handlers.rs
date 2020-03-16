use chrono::prelude::*;
use chrono::{NaiveDateTime};
use diesel::prelude::*;
use itertools::Itertools;
use rocket::Request;
use rocket::response::status;
use rocket_contrib::json::Json;
use rocket::Outcome;
use rocket::http::Status;
use rocket::request::{self, FromRequest};
use rocket_contrib::templates::Template;

use super::schema;
use super::lib::*;
use super::models::*;
use super::measurement::*;


#[get("/")]
pub fn index() -> Template {
    let context = Context {title: "Index".to_string(), lang: "en".to_string()};
    Template::render("index", &context)
}

#[get("/api/v1/measurements")]
pub fn api_get_measurements() -> Json<Vec<Measurement>> {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        .limit(100)
        .load::<Measurement>(&connection)
        .expect("Error fetching measurements");
    Json(results)
}

#[get("/api/v1/measurement/<mid>")]
pub fn api_get_measurement(mid: i32) -> Option<Json<PayloadMeasurement>> {
    use schema::measurements::dsl::*;
    let connection = establish_connection();

    if let Ok(m) = measurements
        .find(mid)
        .get_result::<Measurement>(&connection) {
            if let Ok(mpairs) = MeasurementPair::belonging_to(&m)
                .load(&connection) {
                    let mpairs = mpairs
                        .into_iter()
                        .map(|x: MeasurementPair| RawMeasurementPair { value_type: x.value_type,
                                                                       value: x.value_value })
                        .collect::<Vec<RawMeasurementPair>>();
                    return  Some(Json(PayloadMeasurement {
                        timestamp: m.created_at,
                        lab: m.lab.to_string(),
                        sensor: m.sensor.to_string(),
                        pin: m.pin,
                        software_version: m.software_version.to_string(),
                        sensordatavalues: mpairs
                    }));
                }
        }
    None
}

#[get("/api/v1/labs")]
pub fn api_get_labs() -> Json<Vec<String>> {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        .load::<Measurement>(&connection)
        .expect("Error fetching measurements");

    let results = results
        .into_iter()
        .unique_by(|m| m.lab.to_string())
        .map(|m| m.lab)
        .collect();
    Json(results)
}

#[get("/api/v1/sensors")]
pub fn api_get_sensors() -> Json<Vec<String>> {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        .load::<Measurement>(&connection)
        .expect("Error fetching measurements");

    let results = results
        .into_iter()
        .unique_by(|m| m.sensor.to_string())
        .map(|m| m.sensor)
        .collect();
    Json(results)
}

#[get("/api/v1/phenomena")]
pub fn api_get_phenomena() -> Json<Vec<String>> {
    use schema::measurement_pairs::dsl::*;

    let connection = establish_connection();
    let results = measurement_pairs
        .load::<MeasurementPair>(&connection)
        .expect("Error fetching measurements");

    let results = results
        .into_iter()
        .unique_by(|m| m.value_type.to_string())
        .map(|m| m.value_type)
        .collect();
    Json(results)
}

#[get("/api/v1/lab/<lab_id>")]
pub fn api_get_lab(lab_id: String) -> Json<Vec<Measurement>> {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        .filter(lab.eq(lab_id))
        .limit(100)
        .load::<Measurement>(&connection)
        .expect("Error fetching measurements");

    Json(results)
}

#[get("/api/v1/sensor/<sensor_id>")]
pub fn api_get_sensor(sensor_id: String) -> Json<Vec<Measurement>> {
    use schema::measurements::dsl::*;

    let connection = establish_connection();
    let results = measurements
        .filter(sensor.eq(sensor_id))
        .limit(100)
        .load::<Measurement>(&connection)
        .expect("Error fetching measurements");

    Json(results)
}

#[get("/api/v1/phenomenon/<phen_id>")]
pub fn api_get_phenomenon(phen_id: String) -> Json<Vec<MeasurementPair>> {
    use schema::measurement_pairs::dsl::*;

    let connection = establish_connection();
    let results = measurement_pairs
        .filter(value_type.eq(phen_id))
        .limit(100)
        .load::<MeasurementPair>(&connection)
        .expect("Error fetching measurements");

    Json(results)
}

fn is_valid(id: &str) -> bool {
    id != ""
}

impl<'a, 'r> FromRequest<'a, 'r> for WaterLab {
    type Error = WaterLabError;

    fn from_request(request: &'a Request<'r>) -> request::Outcome<Self, Self::Error> {
        let keys: Vec<_> = request.headers().get("X-Sensor").collect();
        match keys.len() {
            0 => Outcome::Failure((Status::BadRequest, WaterLabError::Missing)),
            1 if is_valid(keys[0]) => Outcome::Success(WaterLab(keys[0].to_string())),
            _ => Outcome::Failure((Status::BadRequest, WaterLabError::Empty)),
        }
    }
}

fn is_valid_pin(pin: &str) -> bool {
    if let Ok(_p) = pin.parse::<i32>() {
        return true
    }
    false
}

impl<'a, 'r> FromRequest<'a, 'r> for SensorPin {
    type Error = SensorPinError;

    fn from_request(request: &'a Request<'r>) -> request::Outcome<Self, Self::Error> {
        let keys: Vec<_> = request.headers().get("X-Pin").collect();
        match keys.len() {
            0 => Outcome::Failure((Status::BadRequest, SensorPinError::Missing)),
            1 if is_valid_pin(keys[0]) => Outcome::Success(SensorPin(keys[0].parse::<i32>().unwrap())),
            _ => Outcome::Failure((Status::BadRequest, SensorPinError::NaN)),
        }
    }
}


#[post("/api/v1/post-sensor-data", format="json", data="<measurement>")]
pub fn handle_send_data(measurement: Json<RawMeasurement>, waterlabid: WaterLab, pin: SensorPin) -> status::Accepted<String> {
    let connection = establish_connection();

    println!(measurement);

    let utc: DateTime<Utc> = Utc::now();
    let ndt: NaiveDateTime = utc.naive_utc();

    if let Some(m) = create_measurement(&connection,
                                        ndt,
                                        &waterlabid.0,
                                        &measurement.sensor,
                                        pin.0,
                                        &measurement.software_version) {
        for x in measurement.sensordatavalues.iter() {
            create_measurement_pair(&connection,
                                    m.id,
                                    &x.value_type,
                                    x.value).expect("Error saving pair");
        }
        status::Accepted(Some(format!("Saved measurement id: '{}'", m.id)))
    } else {
        println!("Error saving measurement");
        status::Accepted(Some("There was an error saving".to_string()))
    }
}
