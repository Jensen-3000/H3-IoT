#include <DHT11.h>
#include <Bounce2.h>
#include <SimpleTimer.h>
#include <EEPROM.h>

// Pins
const int RED_LED_PIN = 6;
const int GREEN_LED_PIN = 5;
const int BLUE_LED_PIN = 4;
const int YELLOW_LED_PIN = 7;
const int BUTTON_PIN = 10;
const int POTENTIOMETER_PIN = A0;

// DHT11 Sensor
DHT11 temperatureHumiditySensor(2);

// Yellow LED state
bool isYellowLEDOn = false;

// EEPROM to store yellow LED state
const int EEPROM_YELLOW_LED_STATE_ADDR = 0;

// Potentiometer range
const int POTENTIOMETER_MIN_VALUE = 0;
const int POTENTIOMETER_MAX_VALUE = 1023;

// Temperature range for mapping
const int MIN_TARGET_TEMPERATURE = 20;
const int MAX_TARGET_TEMPERATURE = 30;
const int TEMPERATURE_OFFSET = 1;

// Button debouncer
Bounce buttonDebouncer;

// Timer for displaying sensor
SimpleTimer temperatureReadTimer(1000);

void setup()
{
    Serial.begin(9600);

    setupPins();
    setupButton();
    loadYellowLEDStateFromEEPROM();

    Serial.println("Setup complete.");
}

void loop()
{
    buttonDebouncer.update();
    checkButtonPress();
    readTemperaturePeriodically();
}

void setupPins()
{
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void setupButton()
{
    buttonDebouncer.attach(BUTTON_PIN);
    buttonDebouncer.interval(25);
}

void loadYellowLEDStateFromEEPROM()
{
    isYellowLEDOn = EEPROM.read(EEPROM_YELLOW_LED_STATE_ADDR);
    updateYellowLEDState();
}

void checkButtonPress()
{
    if (buttonDebouncer.fell())
    {
        toggleYellowLEDState();
    }
}

void toggleYellowLEDState()
{
    isYellowLEDOn = !isYellowLEDOn;
    EEPROM.write(EEPROM_YELLOW_LED_STATE_ADDR, isYellowLEDOn);
    updateYellowLEDState();
}

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

void turnOnYellowLED()
{
    turnOffAllLEDs();
    digitalWrite(YELLOW_LED_PIN, HIGH);
    Serial.println("Yellow LED is ON.");
}

void turnOffYellowLED()
{
    digitalWrite(YELLOW_LED_PIN, LOW);
    Serial.println("Yellow LED is OFF.");
}

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

void processTemperatureAndHumidity(int temperature, int humidity)
{
    int targetTemperature = getTargetTemperatureFromPotentiometer();
    displaySensorReadings(temperature, humidity, targetTemperature);
    controlLEDsBasedOnTemperature(temperature, targetTemperature);
}

int getTargetTemperatureFromPotentiometer()
{
    int potentiometerValue = analogRead(POTENTIOMETER_PIN);
    return map(potentiometerValue, POTENTIOMETER_MIN_VALUE, POTENTIOMETER_MAX_VALUE, MIN_TARGET_TEMPERATURE, MAX_TARGET_TEMPERATURE);
}

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

void turnOffAllLEDs()
{
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(BLUE_LED_PIN, LOW);
    digitalWrite(YELLOW_LED_PIN, LOW);
}

void controlLEDsBasedOnTemperature(int temperature, int targetTemperature)
{
    turnOffAllLEDs();
    if (temperature < targetTemperature - TEMPERATURE_OFFSET)
    {
        digitalWrite(RED_LED_PIN, HIGH); // Heating
    }
    else if (temperature > targetTemperature + TEMPERATURE_OFFSET)
    {
        digitalWrite(BLUE_LED_PIN, HIGH); // Cooling
    }
    else
    {
        digitalWrite(GREEN_LED_PIN, HIGH); // Optimal temperature
    }
}

void handleTemperatureSensorError(int errorCode)
{
    Serial.print("DHT11 Error: ");
    Serial.println(DHT11::getErrorString(errorCode));
}