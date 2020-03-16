handy curl for emulating sensor post

```
curl -X POST https://wasser.datenschule.de/api/v1/post-sensor-data --header "Content-Type:application/json" -H "X-Sensor: foo" -H "X-Pin: 9" -d "{\"sensor\": \"test\", \"software_version\": \"0.0.test\", \"sensordatavalues\": [{\"value_type\": \"test\", \"value\": 50.8}]}"
```
