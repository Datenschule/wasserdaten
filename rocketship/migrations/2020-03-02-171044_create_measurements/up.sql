-- Your SQL goes here
CREATE TABLE measurements (
id SERIAL PRIMARY KEY,
created_at TIMESTAMP NOT NULL,
lab VARCHAR NOT NULL,
sensor VARCHAR NOT NULL,
pin INT NOT NULL,
software_version VARCHAR NOT NULL
)
