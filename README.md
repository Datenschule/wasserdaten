# OpenWaterData

Open source, open hardware prototype for water quality measurement.

It is inspired by [Sensor.community](https://sensor.community/en/) and the actual prototype is based on the [Simple Wate Sensor Platform by Public Lab](https://publiclab.org/notes/wmacfarl/01-10-2020/building-the-simple-water-sensor-platform).


## What does this repo contain?

Each folder contains its own README for further documentation.

- **arduino** Code for the Arduino Uno. Reading sensor data, computing values and communication with nodeMCU
- **nodemcu** Code for the nodeMCU / esp8266. Communicating with the Arduino, connecting to WiFI, sending data to the server.
- **sinatraserver** small Ruby / Sinatra server to quickly debug POST requests from the nodeMCU
- **rocketship** Rust API server and database. This can receive payloads from the nodeMCU, save it to a postgres database and exposes a simple JSON API
- **sensor-calibration-sketches** Calibration and example sketches for the individual water quality sensors, according to the production wiki.
