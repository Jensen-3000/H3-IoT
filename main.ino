/**
 * @file main.ino
 * @brief Temperature and humidity monitoring system with LED indicators
 * @author Jensen-3000
 * @author Mr. GPT
 * @version 1.0
 * @date 2024
 */

/**
 * @mainpage Temperature Monitoring System
 * @brief Monitors temperature and humidity using a DHT11 sensor and provides visual feedback through LEDs.
 *
 * @section description Description
 * Monitors temperature and humidity using a DHT11 sensor and provides visual feedback through LEDs. 
 * Includes temperature control simulation with a target temperature set via potentiometer.
 *
 * @section features Key Features
 * - Temperature and humidity monitoring using DHT11 sensor
 * - Visual feedback through RGB LEDs for temperature status
 * - Target temperature adjustment via potentiometer
 * - System on/off using a button
 * - System state persistence using EEPROM
 *
 * @section hardware Hardware Requirements
 * - Arduino Mega 2560
 * - DHT11 temperature/humidity sensor with 220Ω resistor
 * - 4 LEDs (Red, Green, Blue, Yellow) with 220Ω resistors
 * - 10k potentiometer
 * - Push button
 *
 * @section libraries Libraries Used
 * - Bounce2: https://github.com/thomasfredericks/Bounce2
 * - DHT11: https://github.com/dhrubasaha08/DHT11
 * - SimpleTimer: https://github.com/kiryanenko/SimpleTimer
 */

#include <DHT11.h>
#include <Bounce2.h>
#include <SimpleTimer.h>
#include <EEPROM.h>

/**
 * @defgroup pins Pin Definitions
 * @brief Pin assignments for all hardware connections
 * @{
 */
const int RED_LED_PIN = 6;        ///< Pin for red LED (heating)
const int GREEN_LED_PIN = 5;      ///< Pin for green LED (optimal temperature)
const int BLUE_LED_PIN = 4;       ///< Pin for blue LED (cooling)
const int YELLOW_LED_PIN = 7;     ///< Pin for yellow LED (system on/off)
const int BUTTON_PIN = 10;        ///< Pin for on/off button
const int DHT_SENSOR_PIN = 2;     ///< Pin for DHT11 sensor
const int POTENTIOMETER_PIN = A0; ///< Analog pin for temperature setting
/** @} */

/**
 * @defgroup timing_constants Timing Configuration
 * @brief System timing parameters
 * @{
 */
const int TEMPERATURE_READ_INTERVAL_MS = 1000; ///< Interval between temperature readings
const int DEBOUNCE_INTERVAL_MS = 25;           ///< Button debounce interval
/** @} */

/**
 * @defgroup temperature_config Temperature Configuration
 * @brief Parameters and limits for temperature control
 * @{
 */
const int POTENTIOMETER_MIN_VALUE = 0;    ///< Minimum analog reading from potentiometer
const int POTENTIOMETER_MAX_VALUE = 1023; ///< Maximum analog reading from potentiometer
const int MIN_TARGET_TEMPERATURE = 20;    ///< Minimum settable temperature in Celsius
const int MAX_TARGET_TEMPERATURE = 30;    ///< Maximum settable temperature in Celsius
const int TEMPERATURE_OFFSET = 1;         ///< Temperature offset for the Green LED
/** @} */

/**
 * @defgroup system_state System State and Objects
 * @brief Global state variables and object instances
 * @{
 */
bool isYellowLEDOn = false;                                     ///< Current state of yellow LED
const int EEPROM_YELLOW_LED_STATE_ADDR = 0;                     ///< EEPROM address for storing LED state
DHT11 temperatureHumiditySensor(DHT_SENSOR_PIN);                ///< DHT11 sensor instance
SimpleTimer temperatureReadTimer(TEMPERATURE_READ_INTERVAL_MS); ///< Timer for sensor reading
Bounce buttonDebouncer;                                         ///< Debouncer instance for system button
/** @} */

/**
 * @brief Initialize the system
 * @details Sets up serial communication, configures pins, initializes button debouncer, and loads the previous yellow LED state from EEPROM.
 *
 * @note Serial communication is initialized at 9600 baud rate
 */
void setup()
{
    Serial.begin(9600);
    setupPins();
    setupButton();
    loadYellowLEDStateFromEEPROM();
    Serial.println("Setup complete.");
}

/**
 * @brief Main program loop
 * @details Continuously handles button state updates, button press detection, and temperature reading.
 */
void loop()
{
    buttonDebouncer.update();
    checkButtonPress();
    readTemperaturePeriodically();
}

/**
 * @brief Configures pin modes for LEDs and button
 * @details Sets LED pins as outputs and the button pin as input using the internal pull-up resistor.
 */
void setupPins()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

/**
 * @brief Initialize button debouncer
 * @details Uses Bounce2 for button debouncing to prevent false triggers.
 */
void setupButton()
{
    buttonDebouncer.attach(BUTTON_PIN);
    buttonDebouncer.interval(DEBOUNCE_INTERVAL_MS);
}

/**
 * @brief Load yellow LED state from EEPROM and update LED accordingly
 */
void loadYellowLEDStateFromEEPROM()
{
    isYellowLEDOn = EEPROM.read(EEPROM_YELLOW_LED_STATE_ADDR);
    updateYellowLEDState();
}

/**
 * @brief Check for button press to toggle the Yellow LED state
 * @details Uses Bounce2 for debouncing and triggers the yellow LED toggle on button press.
 */
void checkButtonPress()
{
    if (buttonDebouncer.fell())
    {
        toggleYellowLEDState();
    }
}

/**
 * @brief Toggles the yellow LED state
 * @details Simulates turning the system on/off by toggling the yellow LED state, writes the new state to EEPROM, and updates the LED.
 */
void toggleYellowLEDState()
{
    isYellowLEDOn = !isYellowLEDOn;
    EEPROM.write(EEPROM_YELLOW_LED_STATE_ADDR, isYellowLEDOn);
    updateYellowLEDState();
}

/**
 * @brief Update yellow LED output state
 * @details Sets the yellow LED based on isYellowLEDOn state.
 */
void updateYellowLEDState()
{
    if (isYellowLEDOn)
    {
        turnOnYellowLED();
    }
    else
    {
        turnOffYellowLED();
    }
}

/**
 * @brief Turn on yellow LED and disable other indicators
 * @details Ensures all other LEDs are off before enabling yellow LED.
 */
void turnOnYellowLED()
{
    turnOffAllLEDs();
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Serial.println("Yellow LED is ON.");
}

/**
 * @brief Turn off yellow LED
 * @details Disables the yellow LED and logs the state change.
 */
void turnOffYellowLED()
{
    digitalWrite(YELLOW_LED_PIN, LOW);
    Serial.println("Yellow LED is OFF.");
}

/**
 * @brief Schedule temperature readings
 * @details Uses SimpleTimer to schedule temperature readings at a fixed interval. Only if the system (yellow LED) is on.
 */
void readTemperaturePeriodically()
{
    if (temperatureReadTimer.isReady())
    {
        if (!isYellowLEDOn)
        {
            readAndProcessTemperatureAndHumidity();
        }
        temperatureReadTimer.reset();
    }
}

/**
 * @brief Read and process temperature sensor data
 * @details Attempts to read temperature and humidity from DHT11 sensor and processes the data if successful.
 */
void readAndProcessTemperatureAndHumidity()
{
    int temperature;
    int humidity;
    int result = temperatureHumiditySensor.readTemperatureHumidity(temperature, humidity);

    if (result == 0)
    {
        processTemperatureAndHumidity(temperature, humidity);
    }
    else
    {
        handleTemperatureSensorError(result);
    }
}

/**
 * @brief Process temperature and humidity readings
 * @details Retrieves target temperature, displays readings, and controls LED indicators.
 *
 * @param temperature Current temperature reading in Celsius
 * @param humidity Current humidity reading in percentage
 */
void processTemperatureAndHumidity(int temperature, int humidity)
{
    int targetTemperature = getTargetTemperatureFromPotentiometer();
    displaySensorReadings(temperature, humidity, targetTemperature);
    controlLEDsBasedOnTemperature(temperature, targetTemperature);
}

/**
 * @brief Get target temperature from potentiometer reading
 * @details Maps the analog potentiometer value to a temperature range defined by MIN_TARGET_TEMPERATURE and MAX_TARGET_TEMPERATURE.
 *
 * @return int Target temperature in Celsius
 */
int getTargetTemperatureFromPotentiometer()
{
    int potentiometerValue = analogRead(POTENTIOMETER_PIN);
    return map(potentiometerValue, POTENTIOMETER_MIN_VALUE, POTENTIOMETER_MAX_VALUE, MIN_TARGET_TEMPERATURE, MAX_TARGET_TEMPERATURE);
}

/**
 * @brief Display sensor readings on serial monitor
 * @details Formats and outputs the current temperature, humidity, and target temperature.
 *
 * @param temperature Current temperature in Celsius
 * @param humidity Current humidity percentage
 * @param targetTemperature Target temperature in Celsius, set by the potentiometer
 */
void displaySensorReadings(int temperature, int humidity, int targetTemperature)
{
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C ");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" % ");
    Serial.print("Target Temperature: ");
    Serial.print(targetTemperature);
    Serial.println(" °C ");
}

/**
 * @brief Turn off all LED indicators
 */
void turnOffAllLEDs()
{
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
}

/**
 * @brief Control LED indicators based on current vs target temperature
 * @details Indicates:
 *          - Red: Heating (current < target - offset)
 *          - Blue: Cooling (current > target + offset)
 *          - Green: Optimal (within offset range of target)
 *
 * @param temperature Current measured temperature in Celsius
 * @param targetTemperature Desired temperature in Celsius
 */
void controlLEDsBasedOnTemperature(int temperature, int targetTemperature)
{
    turnOffAllLEDs();
    if (temperature < targetTemperature - TEMPERATURE_OFFSET)
    {
        digitalWrite(RED_LED_PIN, HIGH);
    }
    else if (temperature > targetTemperature + TEMPERATURE_OFFSET)
    {
        digitalWrite(BLUE_LED_PIN, HIGH);
    }
    else
    {
        digitalWrite(GREEN_LED_PIN, HIGH);
    }
}

/**
 * @brief Handle and display DHT11 sensor errors
 * @details Translates error codes to human-readable messages and displays them.
 *
 * @param errorCode Error code from DHT11 sensor reading operation
 */
void handleTemperatureSensorError(int errorCode)
{
    Serial.print("DHT11 Error: ");
    Serial.println(DHT11::getErrorString(errorCode));
}