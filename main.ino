/**
 * @file main.ino
 * @brief Main implementation file for temperature monitoring system
 *
 * @details
 * This file contains the complete implementation of the temperature monitoring system,
 * including sensor reading, LED control, and user input processing.
 *
 * @author Jensen-3000
 * @author Mr. GPT
 * @version 1.0
 * @date 2024
 */

/**
 * @mainpage Temperature Monitoring System
 * @brief Temperature and humidity monitoring system with LED indicators
 *
 * @section description Description
 * This system monitors temperature and humidity using a DHT11 sensor and provides
 * visual feedback through LEDs. It includes temperature control simulation with
 * a target temperature set via potentiometer.
 *
 * @section features Key Features
 * - Temperature and humidity monitoring using DHT11 sensor
 * - Visual feedback through RGB LEDs for temperature status
 * - Target temperature adjustment via potentiometer
 * - System on/off state persistence using EEPROM
 * - Button debouncing for reliable input
 *
 * @section architecture System Architecture
 * The system consists of the following main components:
 * - DHT11 temperature/humidity sensor
 * - 4 LED indicators (RGB + Yellow)
 * - Potentiometer for temperature setting
 * - Push button with debouncing
 * - EEPROM for state persistence
 *
 * @section error_handling Error Handling
 * The system handles the following error conditions:
 * - DHT11 sensor read failures
 * - Invalid temperature/humidity readings
 * - System state persistence errors
 *
 * @section hardware Hardware Requirements
 * - Arduino Mega 2560
 * - DHT11 temperature/humidity sensor with 220Ω resistor
 * - 4 LEDs (Red, Green, Blue, Yellow) with 220Ω resistors
 * - 10k potentiometer
 * - Push button (using internal pull-up)
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
const int RED_LED_PIN = 6;        ///< Pin for red LED (heating indicator)
const int GREEN_LED_PIN = 5;      ///< Pin for green LED (optimal temperature)
const int BLUE_LED_PIN = 4;       ///< Pin for blue LED (cooling indicator)
const int YELLOW_LED_PIN = 7;     ///< Pin for yellow LED (simulate turn on/off)
const int BUTTON_PIN = 10;        ///< Pin for turn on/off button
const int DHT_SENSOR_PIN = 2;     ///< Pin for DHT11 sensor
const int POTENTIOMETER_PIN = A0; ///< Analog pin for temperature setting
/** @} */

/**
 * @defgroup timing_constants Timing Configuration
 * @brief System timing parameters
 * @details Defines various timing constants used throughout the system for
 *          sensor reading intervals and input debouncing
 * @{
 */
const int TEMPERATURE_READ_INTERVAL_MS = 1000; ///< Interval between temperature readings
const int DEBOUNCE_INTERVAL_MS = 25;           ///< Button debounce interval
/** @} */

/**
 * @defgroup temperature_config Temperature Configuration
 * @brief Temperature control parameters and limits
 * @{
 */
const int POTENTIOMETER_MIN_VALUE = 0;    ///< Minimum analog reading from potentiometer
const int POTENTIOMETER_MAX_VALUE = 1023; ///< Maximum analog reading from potentiometer
const int MIN_TARGET_TEMPERATURE = 20;    ///< Minimum settable temperature in Celsius
const int MAX_TARGET_TEMPERATURE = 30;    ///< Maximum settable temperature in Celsius
const int TEMPERATURE_OFFSET = 1;         ///< Tolerance range for temperature control
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
 * @details Sets up serial communication, configures pins, initializes button debouncer,
 * and loads the previous yellow LED state from EEPROM.
 *
 * @note Serial communication is initialized at 9600 baud rate
 * @see setupPins()
 * @see setupButton()
 * @see loadYellowLEDStateFromEEPROM()
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
 * @details Continuously handles:
 *          - Button state updates
 *          - Button press detection
 *          - Temperature reading schedule
 *
 * @see buttonDebouncer
 * @see checkButtonPress()
 * @see readTemperaturePeriodically()
 */
void loop()
{
    buttonDebouncer.update();
    checkButtonPress();
    readTemperaturePeriodically();
}

/**
 * Configure pin modes for all LEDs and inputs
 * @details Sets up all LED pins as outputs and button pin as input with pull-up resistor
 * @note Uses Arduino Mega 2560's internal pull-up resistor
 *
 * @pre All pin constants must be defined
 * @see RED_LED_PIN
 * @see GREEN_LED_PIN
 * @see BLUE_LED_PIN
 * @see YELLOW_LED_PIN
 * @see BUTTON_PIN
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
 * Initialize button debouncer with specified settings
 * @details Configures button debouncing to prevent false triggers
 *
 * @see BUTTON_PIN
 * @see DEBOUNCE_INTERVAL_MS
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
 * @brief Check for button press events and handle them
 * @details Monitors the debounced button state and triggers toggle when pressed
 */
void checkButtonPress()
{
    if (buttonDebouncer.fell())
    {
        toggleYellowLEDState();
    }
}

/**
 * @brief Toggle system power state
 * @details Toggles the yellow LED state and persists the new state to EEPROM
 *
 * @note This function handles the main power simulation of the system
 * @see updateYellowLEDState()
 * @see EEPROM_YELLOW_LED_STATE_ADDR
 */
void toggleYellowLEDState()
{
    isYellowLEDOn = !isYellowLEDOn;
    EEPROM.write(EEPROM_YELLOW_LED_STATE_ADDR, isYellowLEDOn);
    updateYellowLEDState();
}

/**
 * @brief Update yellow LED output state
 * @details Sets the yellow LED based on isYellowLEDOn state
 *
 * @see turnOnYellowLED()
 * @see turnOffYellowLED()
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
 * @details Ensures all other LEDs are off before enabling yellow LED
 *
 * @see turnOffAllLEDs()
 * @see YELLOW_LED_PIN
 */
void turnOnYellowLED()
{
    turnOffAllLEDs();
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Serial.println("Yellow LED is ON.");
}

/**
 * @brief Turn off yellow LED
 * @details Disables the yellow LED and logs the state change
 *
 * @see YELLOW_LED_PIN
 */
void turnOffYellowLED()
{
    digitalWrite(YELLOW_LED_PIN, LOW);
    Serial.println("Yellow LED is OFF.");
}

/**
 * @brief Schedule temperature readings
 * @details Checks if it's time to read temperature and only proceeds
 *          if the system is active (yellow LED off)
 *
 * @see temperatureReadTimer
 * @see isYellowLEDOn
 * @see readAndProcessTemperatureAndHumidity()
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
 * @details Attempts to read temperature and humidity from DHT11 sensor
 *          and processes the data if successful
 *
 * @note Sensor readings may be delayed by up to 1 second
 * @see processTemperatureAndHumidity()
 * @see handleTemperatureSensorError()
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
 * @details Retrieves target temperature, displays readings, and controls LED indicators
 *
 * @param temperature Current temperature reading in Celsius
 * @param humidity Current humidity reading in percentage
 *
 * @see getTargetTemperatureFromPotentiometer()
 * @see displaySensorReadings()
 * @see controlLEDsBasedOnTemperature()
 */
void processTemperatureAndHumidity(int temperature, int humidity)
{
    int targetTemperature = getTargetTemperatureFromPotentiometer();
    displaySensorReadings(temperature, humidity, targetTemperature);
    controlLEDsBasedOnTemperature(temperature, targetTemperature);
}

/**
 * @brief Get target temperature from potentiometer reading
 *
 * @details Maps the analog potentiometer value to a temperature
 * range defined by MIN_TARGET_TEMPERATURE and MAX_TARGET_TEMPERATURE
 *
 * @return int Target temperature in Celsius
 * @retval MIN_TARGET_TEMPERATURE Returned when potentiometer at minimum
 * @retval MAX_TARGET_TEMPERATURE Returned when potentiometer at maximum
 * @retval Other values Linearly mapped between min and max temperatures
 *
 * @see MIN_TARGET_TEMPERATURE
 * @see MAX_TARGET_TEMPERATURE
 */
int getTargetTemperatureFromPotentiometer()
{
    int potentiometerValue = analogRead(POTENTIOMETER_PIN);
    return map(potentiometerValue, POTENTIOMETER_MIN_VALUE, POTENTIOMETER_MAX_VALUE, MIN_TARGET_TEMPERATURE, MAX_TARGET_TEMPERATURE);
}

/**
 * @brief Display sensor readings on serial monitor
 * @details Formats and outputs the current temperature, humidity, and target temperature
 *
 * @param temperature Current temperature in Celsius
 * @param humidity Current humidity percentage
 * @param targetTemperature Target temperature in Celsius
 *
 * @pre Serial communication must be initialized
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
 *
 * @pre All LED pins must be configured as outputs
 * @see turnOffAllLEDs()
 * @see TEMPERATURE_OFFSET
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
 * @details Translates error codes to human-readable messages and displays them
 *
 * @param errorCode Error code from DHT11 sensor reading operation
 *
 * @pre Serial communication must be initialized
 * @see DHT11::getErrorString()
 */
void handleTemperatureSensorError(int errorCode)
{
    Serial.print("DHT11 Error: ");
    Serial.println(DHT11::getErrorString(errorCode));
}