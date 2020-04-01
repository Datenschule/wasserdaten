# Sinatraserver

This is a very simple ruby server to check the payloads that the nodemcu is sending.

The only endpoint of interest is `POST /post-sensor-data` where data should be sent to.
It accepts any JSON payload and will simply parse and print the payload to the console. It returns 'thanks'.

After installing dependencies once with `$ bundle install`, start the server with

``` bash
$ bundle exec thin start
```
