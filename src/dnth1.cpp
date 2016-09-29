/*

 Firmware for DN-TH1
 Author: Carlos Fernando Avila Gratz

 Depends on:
 - Adafruit_DHT_Unified

*/
#include <DHT_U.h>

// pins and devices
#define RELAY1      6
#define RELAY2      5
#define RELAY3      4
#define RELAY4      3
#define ERROR_LED   13
#define LDR_LED     A0
#define DHTPIN      7

// params
#define IDEAL_TEMP_DAY    27.0
#define IDEAL_TEMP_NIGHT  22.0
#define IDEAL_HUMI        63.0
#define MAX_READ_ERRORS   5


#define ARCOND       RELAY1
#define DEHUMIDIFIER RELAY3
#define HUMIDIFIER   RELAY4
#define TURN_ON     HIGH
#define TURN_OFF    LOW

// device handles
DHT_Unified dht(DHTPIN, DHT22);
int readErrors = 0;

void blink_error() {

    Serial.print("Error!\n");
    for (int i = 0; i < 3; i++) {
        digitalWrite(ERROR_LED, HIGH);
        delay(50);
        digitalWrite(ERROR_LED, LOW);
        delay(50);
    }
}

bool is_daytime() {
    return analogRead(LDR_LED) > 100;
}



void controlTemperature() {

    sensors_event_t event;
    dht.temperature().getEvent(&event);

    if (isnan(event.temperature)) {
        Serial.print("Error reading temperature.\n");
        blink_error();
        readErrors++;

        // if the error persists (broken sensor), keep de AC on!
        if (readErrors > MAX_READ_ERRORS) {
            digitalWrite(ARCOND, TURN_ON);
        }

        return;
    }

    float temp   = event.temperature;
    float IDEAL_TEMP = is_daytime() ? IDEAL_TEMP_DAY : IDEAL_TEMP_NIGHT;
    float offset = temp - IDEAL_TEMP;

    Serial.print("T: ");
    Serial.print(temp);
    Serial.print("º (");
    if (offset > 0) Serial.print('+');
    Serial.print(offset);
    Serial.print("º)\n");


    if (temp > (IDEAL_TEMP + 1) && digitalRead(ARCOND) == TURN_OFF) {
        Serial.print("Turning AC: ON\n");
        digitalWrite(ARCOND, TURN_ON);
    }

    if (temp < (IDEAL_TEMP - 1) && digitalRead(ARCOND) == TURN_ON) {
        Serial.print("Turning AC: OFF\n");
        digitalWrite(ARCOND, TURN_OFF);
    }
}

void controlHumidity() {

    sensors_event_t event;
    dht.humidity().getEvent(&event);

    if (isnan(event.relative_humidity)) {
        Serial.print("Error reading humidity.\n");
        blink_error();
        return;
    }

    float humidity = event.relative_humidity;
    float offset = humidity - IDEAL_HUMI;

    Serial.print("H: ");
    Serial.print(humidity);
    Serial.print("% (");
    if (offset > 0) Serial.print('+');
    Serial.print(offset);
    Serial.print("%)\n");

    // DEHUMIDIFIER
    if (humidity > (IDEAL_HUMI + 1) && digitalRead(DEHUMIDIFIER) == TURN_OFF) {

        Serial.print("Turning DEHUMIDIFIER: ON\n");
        digitalWrite(DEHUMIDIFIER, TURN_ON);
    }

    if (humidity < (IDEAL_HUMI - 1) && digitalRead(DEHUMIDIFIER) == TURN_ON) {

        Serial.print("Turning DEHUMIDIFIER: OFF\n");
        digitalWrite(DEHUMIDIFIER, TURN_OFF);
    }

    // HUMIDIFIER
    if (humidity < (IDEAL_HUMI - 1) && digitalRead(HUMIDIFIER) == TURN_OFF) {

        Serial.print("Turning HUMIDIFIER: ON\n");
        digitalWrite(HUMIDIFIER, TURN_ON);
    }

    if (humidity > (IDEAL_HUMI + 1) && digitalRead(HUMIDIFIER) == TURN_ON) {

        Serial.print("Turning HUMIDIFIER: OFF\n");
        digitalWrite(HUMIDIFIER, TURN_OFF);
    }
}


void setup() {

    Serial.begin(9600);
    dht.begin();

    pinMode(ARCOND, OUTPUT);
    pinMode(DEHUMIDIFIER, OUTPUT);
    pinMode(HUMIDIFIER, OUTPUT);
    digitalWrite(ARCOND, TURN_OFF);
    digitalWrite(DEHUMIDIFIER, TURN_OFF);
    digitalWrite(HUMIDIFIER, TURN_OFF);
}

void loop() {


    // Temperature
    controlTemperature();
    delay(500);

    // Humidity
    controlHumidity();

    delay(2000);
    Serial.print("\n");
}
