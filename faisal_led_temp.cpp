#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <thread>

// Faisal's Adafruit IO credentials
const std::string ADAFRUIT_IO_USERNAME = "SMFAISAL404";
const std::string ADAFRUIT_IO_KEY = "aio_nGQM985xkxp2UmgSu2Y44XdaGiKl";
const std::string FEED_LED_STATUS = "led-toggle";        // Feed to send LED status (ON/OFF)
const std::string FEED_TEMPERATURE = "room-temperature"; // Feed to read temperature from

// Function to send LED status to Adafruit IO
void sendLEDStatusToAdafruit(const std::string& status) {
    std::string command = "curl -s -X POST "
                          "https://io.adafruit.com/api/v2/" + ADAFRUIT_IO_USERNAME + "/feeds/" + FEED_LED_STATUS +
                          "/data "
                          "-H 'Content-Type: application/json' "
                          "-H 'X-AIO-Key: " + ADAFRUIT_IO_KEY + "' "
                          "-d '{\"value\": \"" + status + "\"}'";

    system(command.c_str());
}

// Export GPIO pin if not already exported
void exportGPIOPin(int gpioPin) {
    std::ifstream checkGPIO("/sys/class/gpio/gpio" + std::to_string(gpioPin) + "/value");
    if (!checkGPIO.good()) {
        std::ofstream exportFile("/sys/class/gpio/export");
        if (exportFile.is_open()) {
            exportFile << gpioPin;
            exportFile.close();
        } else {
            std::cerr << "Error exporting GPIO pin " << gpioPin << std::endl;
        }
    }
}

// Set GPIO direction (in/out)
void setGPIODirection(int gpioPin, const std::string& direction) {
    std::ofstream directionFile("/sys/class/gpio/gpio" + std::to_string(gpioPin) + "/direction");
    if (directionFile.is_open()) {
        directionFile << direction;
        directionFile.close();
    } else {
        std::cerr << "Error setting direction for GPIO pin " << gpioPin << std::endl;
    }
}

// Set GPIO value (0 or 1)
void writeGPIOState(int gpioPin, int value) {
    std::ofstream valueFile("/sys/class/gpio/gpio" + std::to_string(gpioPin) + "/value");
    if (valueFile.is_open()) {
        valueFile << value;
        valueFile.close();
    } else {
        std::cerr << "Error writing to GPIO pin " << gpioPin << std::endl;
    }
}

// Curl callback to store HTTP response in string
size_t curlWriteToString(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Read latest temperature value from Adafruit IO feed
float readTemperatureFromAdafruit() {
    CURL* curl;
    CURLcode result;
    std::string responseData;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        std::string requestUrl = "https://io.adafruit.com/api/v2/" + ADAFRUIT_IO_USERNAME + "/feeds/" + FEED_TEMPERATURE + "/data/last";
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("X-AIO-Key: " + ADAFRUIT_IO_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, requestUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlWriteToString);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        result = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    size_t valueStart = responseData.find("\"value\":\"");
    if (valueStart != std::string::npos) {
        valueStart += 9;
        size_t valueEnd = responseData.find("\"", valueStart);
        std::string tempString = responseData.substr(valueStart, valueEnd - valueStart);
        return std::stof(tempString);
    }

    return -1000.0f; // Return invalid temp on failure
}

int main() {
    const int LED_GPIO_PIN = 60;
    const int TEMPERATURE_THRESHOLD_C = 85;

    exportGPIOPin(LED_GPIO_PIN);
    setGPIODirection(LED_GPIO_PIN, "out");

    std::cout << "Monitoring temperature feed to control LED..." << std::endl;

    while (true) {
        float currentTemperature = readTemperatureFromAdafruit();

        if (currentTemperature == -1000.0f) {
            std::cerr << "Error: Could not read temperature. Retrying..." << std::endl;
        } else {
            if (currentTemperature < TEMPERATURE_THRESHOLD_C) {
                std::cout << "Temperature: " << currentTemperature << "°C -> LED ON" << std::endl;
                writeGPIOState(LED_GPIO_PIN, 1);
                sendLEDStatusToAdafruit("ON");
            } else {
                std::cout << "Temperature: " << currentTemperature << "°C -> LED OFF" << std::endl;
                writeGPIOState(LED_GPIO_PIN, 0);
                sendLEDStatusToAdafruit("OFF");
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}
