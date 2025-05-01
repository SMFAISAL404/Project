// led_control_faisal.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <chrono>
#include <thread>

const std::string ADAFRUIT_IO_KEY = "aio_MWRm265Oj8VshJCdv5naIHgbgS2G";
const std::string ADAFRUIT_IO_USERNAME = "SMFAISAL404";
const std::string FEED_NAME = "led-blink";  // Feed name for number input

// Export GPIO if not already exported
void exportGPIO(int gpio) {
    std::ifstream gpioFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (!gpioFile.good()) {
        std::ofstream exportFile("/sys/class/gpio/export");
        if (exportFile.is_open()) {
            exportFile << gpio;
            exportFile.close();
        } else {
            std::cerr << "Error exporting GPIO" << gpio << std::endl;
        }
    }
}

// Set GPIO direction
void setGPIODirection(int gpio, const std::string& direction) {
    std::ofstream directionFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/direction");
    if (directionFile.is_open()) {
        directionFile << direction;
        directionFile.close();
    } else {
        std::cerr << "Error setting direction for GPIO" << gpio << std::endl;
    }
}

// Write value to GPIO
void writeGPIOValue(int gpio, int value) {
    std::ofstream valueFile("/sys/class/gpio/gpio" + std::to_string(gpio) + "/value");
    if (valueFile.is_open()) {
        valueFile << value;
        valueFile.close();
    } else {
        std::cerr << "Error writing value to GPIO" << gpio << std::endl;
    }
}

// Callback to capture HTTP response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Get blink count from Adafruit IO feed
int getBlinkCountFromFeed() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        std::string url = "https://io.adafruit.com/api/v2/" + ADAFRUIT_IO_USERNAME + "/feeds/" + FEED_NAME + "/data/last";
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, ("X-AIO-Key: " + ADAFRUIT_IO_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    size_t pos = readBuffer.find("\"value\":\"");
    if (pos != std::string::npos) {
        size_t start = pos + 9;
        size_t end = readBuffer.find("\"", start);
        std::string countStr = readBuffer.substr(start, end - start);
        try {
            return std::stoi(countStr);
        } catch (...) {
            std::cerr << "Error converting feed value to integer!" << std::endl;
            return 0;
        }
    }

    return 0;
}

int main() {
    const int ledGPIO = 60; // Onboard LED pin

    exportGPIO(ledGPIO);
    setGPIODirection(ledGPIO, "out");

    std::cout << "Starting LED blink control based on Adafruit feed..." << std::endl;

    while (true) {
        int blinkCount = getBlinkCountFromFeed();

        if (blinkCount <= 0) {
            std::cerr << "Invalid blink count (" << blinkCount << "). Retrying..." << std::endl;
        } else {
            std::cout << "Blinking LED " << blinkCount << " times..." << std::endl;
            for (int i = 0; i < blinkCount; ++i) {
                writeGPIOValue(ledGPIO, 1); // ON
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                writeGPIOValue(ledGPIO, 0); // OFF
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
