// window_monitor_faisal.cpp

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <chrono>
#include <thread>

const std::string ADAFRUIT_IO_KEY = "aio_MWRm265Oj8VshJCdv5naIHgbgS2G";
const std::string FEED_NAME = "button-status";
const std::string ADAFRUIT_IO_USERNAME = "SMFAISAL404";

const int BUTTON_GPIO_NUMBER = 46; // P8_16
const std::string GPIO_FULL_PATH = "/sys/class/gpio/gpio" + std::to_string(BUTTON_GPIO_NUMBER);

// Run a shell command safely
void runShellCommand(const std::string& command) {
    int result = system(command.c_str());
    if (result != 0) {
        std::cerr << "Shell command failed: " << command << std::endl;
    }
}

// Set up GPIO pin as input
void initializeGPIO() {
    std::ifstream checkPath(GPIO_FULL_PATH + "/value");
    if (!checkPath.good()) {
        std::ofstream exportPin("/sys/class/gpio/export");
        exportPin << BUTTON_GPIO_NUMBER;
        exportPin.close();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    std::ofstream direction(GPIO_FULL_PATH + "/direction");
    direction << "in";
    direction.close();

    runShellCommand("config-pin P8_16 gpio_pu");
}

// Read the current state of the GPIO pin
int getButtonState() {
    std::ifstream inputFile(GPIO_FULL_PATH + "/value");
    int buttonState = -1;
    if (inputFile.is_open()) {
        inputFile >> buttonState;
        inputFile.close();
    }
    return buttonState;
}

// Send status to Adafruit IO
void updateAdafruitFeed(const std::string& statusValue) {
    std::string postCommand = "curl -s -X POST "
                              "https://io.adafruit.com/api/v2/" + ADAFRUIT_IO_USERNAME + "/feeds/" + FEED_NAME +
                              "/data "
                              "-H 'Content-Type: application/json' "
                              "-H 'X-AIO-Key: " + ADAFRUIT_IO_KEY + "' "
                              "-d '{\"value\": \"" + statusValue + "\"}'";
    runShellCommand(postCommand);
}

int main() {
    initializeGPIO();
    std::cout << "Monitoring button to detect window status..." << std::endl;

    int lastButtonState = -1;

    while (true) {
        int currentButtonState = getButtonState();

        if (currentButtonState != lastButtonState && (currentButtonState == 0 || currentButtonState == 1)) {
            if (currentButtonState == 0) {
                std::cout << "[Window Closed] Button Pressed" << std::endl;
                updateAdafruitFeed("0");
            } else if (currentButtonState == 1) {
                std::cout << "[Window Open] Button Released" << std::endl;
                updateAdafruitFeed("1");
            }
            lastButtonState = currentButtonState;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    return 0;
}
