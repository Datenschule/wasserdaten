# Rocketship

Rocketship is a barebones API server for storing and reading water measurement data collected by the sensors described in this repository.

It uses [Rocket](https://rocket.rs/) as webframework and [Diesel](https://diesel.rs/) as ORM.


See a running version at https://wasser.datenschule.de/

## Run Rocketship locally

In stall Rust as outlined in [the Rust install guide](https://www.rust-lang.org/tools/install). Note that you need **nightly Rust** as this is required by Rocket.

You also need to install Diesel locally to manage your Postgres database. [Diesel has a nice getting started guide for that](https://diesel.rs/guides/getting-started/).

Once installed, setup the database and run migrations as described in the guide.

Then, to start the server, run

``` bash
$ cargo run --bin server
```

And Rocket should start up the server on `0.0.0.0:8000`.

## Docker setup

The Rocket app comes with a `Dockerfile` and a `docker-compose.yml` for an easy (production?) setup including a postgres db.

`$ docker-compose up`

I haven't figured out how to cache cargo dependencies though, so first build will take some time. Sorry!


## Diesel database utils

There are three other binaries defined to help you interact with the database via Diesel

- `cargo run --bin show_measurement` list all measurements

- `cargo run --bin write_measurement` add a test measurement to the db

- `cargo run --bin delete_measurement <id>` remove a measurement with <id> (and its associated measurement pairs) from the db

These are also available in the Docker container.



## Misc

Handy curl for emulating sensor post

```
curl -X POST 0.0.0.0:8000/api/v1/post-sensor-data --header "Content-Type:application/json" -H "X-Sensor: foo" -H "X-Pin: 9" -d "{\"sensor\": \"test\", \"software_version\": \"0.0.test\", \"sensordatavalues\": [{\"value_type\": \"test\", \"value\": 50.8}]}"
```
