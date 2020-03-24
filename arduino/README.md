# Arduino Uno

We're using the Arduino Uno to collect measurements and communicate them to the nodeMCU.

Using the Arduino IDE, flash this program onto your board.


## Hardware

We currently use an Arduino Uno to talk to the nodeMCU.

For serial communication, connect the pins like so:

| nodeMCU | Arduino Uno |
|---------|-------------|
| D5      | 9           |
| D6      | 8           |

Arduino Uno and nodeMCU need to share a common ground!

Calibrate the sensors as described in the manufacturer's wiki (links in the sketches README).

All sensors are analog, the expected connections are:

| sensor | Arduino Uno |
|  Turbidity | A0           |
| ORP | A1 |
|ph | A2 |
| DO | A3 |
| EC | A4 |


## Software

Note that it's quite hard to figure out if an analog signal is actually connected or not. While there is a little plausibility check in place before serializing the data, natural changes in the current can give fake signals (especially since some of the sensors draw quite a but of current).
If you already know you won't be using a certain sensor in your setup, just comment out the respective code blocks. They should be easy to spot.

### Add additional libraries
This projects relies on third party libraries. You can install all of them through the Library Manager in the Arduino IDE.

- [**AltSoftSerial**](https://github.com/PaulStoffregen/AltSoftSerial) (Note that this is a different library than on the nodeMCU. This one works nicely alongside SoftTimer).
- [**ArduinoJSON**](https://github.com/bblanchon/ArduinoJson)
- [**SoftTimer**](https://github.com/prampec/arduino-softtimer)

- [**DFRobot_EC10**](https://github.com/DFRobot/DFRobot_EC/) (This is a special library for the electronic conductivity sensor)
