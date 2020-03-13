table! {
    measurement_pairs (id) {
        id -> Int4,
        measurement_id -> Int4,
        value_type -> Varchar,
        value_value -> Float4,
    }
}

table! {
    measurements (id) {
        id -> Int4,
        created_at -> Timestamp,
        lab -> Varchar,
        sensor -> Varchar,
        pin -> Int4,
        software_version -> Varchar,
    }
}

allow_tables_to_appear_in_same_query!(
    measurement_pairs,
    measurements,
);
