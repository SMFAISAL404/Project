#include <iostream>
#include <unistd.h>
#include "AnalogIn.h" // Molloy's AnalogIn class for analog voltage input

// Adafruit IO credentials and feed configuration
const std::string AIO_USERNAME = "SMFAISAL404";
const std::string AIO_KEY = "aio_MUvj28bQJqRgzsSmICzJm1bfIjFl";
const std::string FEED_NAME = "room-temperature";

int main() {
    AnalogIn tempSensor(0); // Analog input pin AIN0 (P9_39) used with LM35 sensor

    while (true) {
        float tempValue = tempSensor.getVoltage() * 100; // LM35 outputs 10mV per °C

        std::string apiUrl = "https://io.adafruit.com/api/v2/" + AIO_USERNAME +
                             "/feeds/" + FEED_NAME + "/data?X-AIO-Key=" + AIO_KEY;

        std::string command = "curl -X POST -F \"value=" + std::to_string(tempValue) + "\" \"" + apiUrl + "\"";
        system(command.c_str()); // Send temperature data to Adafruit IO

        sleep(5); // Wait 5 seconds before next reading
    }

    return 0;
}
