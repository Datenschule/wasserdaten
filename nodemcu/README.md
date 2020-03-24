# NodeMCU

We're using the nodeMCU v2 (ESP8266) to read data from the Arduino send data to the server.

Using the Arduino IDE, you can flash this program onto your nodeMCU.

## Hardware

We currently use a nodeMCU v2 to talk to an Arduino Uno.

For serial communication, connect the pins like so:

| nodeMCU | Arduino Uno |
|---------|-------------|
| D5      | 9           |
| D6      | 8           |

Arduino Uno and nodeMCU need to share a common ground!

## Software

### Add the board
You will need to add the nodeMCU board to your Arduino IDE.

Under **Preferences**, paste this url `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into **Additional Boards Manager URLs**.

Next, go to **Tools** > **Boards** > **Board Manager** and search for *esp8266 by esp8266 community* and install the software. You can now use the nodeMCU like any other Arduino board.

### Add additional libraries
This projects relies on third party libraries. You can install all of them through the Library Manager in the Arduino IDE.

- [**ArduinoJSON**](https://github.com/bblanchon/ArduinoJson)
- [**SoftwareSerial** for ESP8266](https://github.com/plerup/espsoftwareserial)
