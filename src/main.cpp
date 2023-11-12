#include <Arduino.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <M5Unified.h>
#include <Avatar.h>

using namespace m5avatar;

Avatar avatar;
SensirionI2CScd4x scd4x;

//Output hexadecimal numbers to serial monitor
void printUint16Hex(uint16_t value) {
    Serial.print(value < 4096 ? "0" : "");
    Serial.print(value < 256 ? "0" : "");
    Serial.print(value < 16 ? "0" : "");
    Serial.print(value, HEX);
}
//Concatenate and display three hexadecimal numbers
void printSerialNumber(uint16_t serial0, uint16_t serial1, uint16_t serial2) {
    Serial.print("Serial: 0x");
    printUint16Hex(serial0);
    printUint16Hex(serial1);
    printUint16Hex(serial2);
    Serial.println();
}

void setup() {
    M5.begin();//Initialization of M5Stack board
    Serial.begin(115200);//Start communication with serial monitor at a communication speed of 115200
    while (!Serial) {
        delay(100);//Wait until serial communication is established
    }

    Wire.begin();//Initialize I2C communication (serial communication protocol)

    uint16_t error;
    char errorMessage[256];

    scd4x.begin(Wire);//Initialization of SCD4x sensor

    // stop potentially previously started measurement
    error = scd4x.stopPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute stopPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    uint16_t serial0;
    uint16_t serial1;
    uint16_t serial2;
    error = scd4x.getSerialNumber(serial0, serial1, serial2);
    if (error) {
        Serial.print("Error trying to execute getSerialNumber(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else {
        printSerialNumber(serial0, serial1, serial2);
    }

    // Start Measurement
    error = scd4x.startPeriodicMeasurement();
    if (error) {
        Serial.print("Error trying to execute startPeriodicMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    }

    Serial.println("Waiting for first measurement... (5 sec)");
}

void loop() {
    uint16_t error;
    char errorMessage[256];

    delay(100);

    // Read Measurement
    uint16_t co2 = 0;
    float temperature = 0.0f;
    float humidity = 0.0f;
    bool isDataReady = false;
    error = scd4x.getDataReadyFlag(isDataReady);
    if (error) {
        Serial.print("Error trying to execute getDataReadyFlag(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
        return;
    }
    if (!isDataReady) {
        return;
    }
    error = scd4x.readMeasurement(co2, temperature, humidity);
    if (error) {
        Serial.print("Error trying to execute readMeasurement(): ");
        errorToString(error, errorMessage, 256);
        Serial.println(errorMessage);
    } else if (co2 == 0) {
        Serial.println("Invalid sample detected, skipping.");
    } else {
        // Check if CO2 concentration is greater than 900 ppm
        if (co2 > 900) {
            // Output to Serial Monitor
            Serial.println("CO2 concentration exceeds 900 ppm!");

            // Output to M5.Lcd
            M5.Lcd.fillScreen(TFT_BLACK);
            M5.Lcd.setTextColor(TFT_RED);  // Change text color to red
            M5.Lcd.setTextSize(2);
            M5.Lcd.setCursor(0, 0);
            M5.Lcd.print("CO2 Concentration:");
            M5.Lcd.setCursor(0, 40);
            M5.Lcd.setTextSize(4);
            M5.Lcd.print(co2);
            M5.Lcd.print(" ppm");
        } else {
            avatar.init();
        }
    }
}
