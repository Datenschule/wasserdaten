# NodeMCU

We're using the nodeMCU circuit board, also known as ESP8266.

Using the Arduino IDE, you can flash this program onto your nodeMCU.

## Hardware

We currently use a nodeMCU v2 with a DHT22 that is writing to pin D7 aka 13.

## Software

### Add the board
You will need to add the nodeMCU board to your Arduino IDE.

Under **Preferences**, paste this url `http://arduino.esp8266.com/stable/package_esp8266com_index.json` into **Additional Boards Manager URLs**.

Next, go to **Tools** > **Boards** > **Board Manager** and search for *esp8266 by esp8266 community* and install the software. You can now use the nodeMCU like any other Arduino board.

### Add additional libraries
This projects relies on a couple of third party libraries. You can install all of them through the Library Manager in the Arduino IDE.

- **NTPClient** by Frabrice Weinberg for fetching accurate times
- **WiFi** by Arduino for working with WiFi
- **ArduinoJSON** by Benoit Blanchon for easily building a JSON payload
- **DHT Sensor Library** by AdaFruit for a nice sensor reading interface
