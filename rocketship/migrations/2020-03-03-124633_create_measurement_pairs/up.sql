CREATE Table measurement_pairs (
id SERIAL PRIMARY KEY,
measurement_id INT NOT NULL,
value_type VARCHAR NOT NULL,
value_value REAL NOT NULL
)
