/*

 Firmware for DN-TH1
 Author: Carlos Fernando Avila Gratz

 Depends on:
 - Adafruit_DHT_Unified

*/
#include <DHT_U.h>

// pins and devices
#define RELAY1      2
#define RELAY2      4
// #define RELAY3      4
// #define RELAY4      3
#define RED_LED   3
#define GREEN_LED   5
#define BLUE_LED   6
#define LDR_LED     A0
#define DHTPIN      7

// params
#define IDEAL_TEMP_DAY    27.0
#define IDEAL_TEMP_NIGHT  22.0
#define IDEAL_HUMI        57.0
#define MAX_READ_ERRORS   5


#define ARCOND       RELAY1
#define DEHUMIDIFIER RELAY2
// #define HUMIDIFIER   RELAY4
#define TURN_ON     HIGH
#define TURN_OFF    LOW

struct device_state {
    bool arcond;
    bool dehumidifier;
} state = {false, false};

// device handles
DHT_Unified dht(DHTPIN, DHT22);
int readErrors = 0;


void blink_error() {

    Serial.print("Error!\n");
    for (int i = 0; i < 3; i++) {
        digitalWrite(RED_LED, HIGH);
        delay(1000);
        digitalWrite(RED_LED, LOW);
    }
}

bool is_daytime() {
    return analogRead(LDR_LED) > 100;
}


void turn_ac_on () {

    if (state.arcond) return;

    Serial.print("Turning AC: ON!\n");
    digitalWrite(ARCOND, TURN_ON);
    state.arcond = true;
}

void turn_ac_off () {
    if (!state.arcond) return;
    digitalWrite(ARCOND, TURN_OFF);
    Serial.print("Turning AC: OFF!\n");
    state.arcond = false;
}

void turn_dehumidifier_on () {
    if (state.dehumidifier)  return;

    digitalWrite(ARCOND, TURN_ON);
    Serial.print("Turning DEHUMIDIFIER: ON!\n");
    state.dehumidifier = true;
}

void turn_dehumidifier_off () {
    if (!state.dehumidifier) return;
    digitalWrite(ARCOND, TURN_OFF);
    Serial.print("Turning DEHUMIDIFIER: OFF!\n");
    state.dehumidifier = false;
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
            turn_ac_on();
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


    if (temp > (IDEAL_TEMP + 1)) {
        turn_ac_on();
    }

    if (temp < (IDEAL_TEMP - 1)) {
        turn_ac_off();
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
    if (humidity > (IDEAL_HUMI + 1) && digitalRead(DEHUMIDIFIER) == TURN_OFF)
        turn_dehumidifier_on();

    if (humidity < (IDEAL_HUMI - 1) && digitalRead(DEHUMIDIFIER) == TURN_ON)
        turn_dehumidifier_off();

    // HUMIDIFIER
    // if (humidity < (IDEAL_HUMI - 1) && digitalRead(HUMIDIFIER) == TURN_OFF) {
    //
    //     Serial.print("Turning HUMIDIFIER: ON\n");
    //     digitalWrite(HUMIDIFIER, TURN_ON);
    // }
    //
    // if (humidity > (IDEAL_HUMI + 1) && digitalRead(HUMIDIFIER) == TURN_ON) {
    //
    //     Serial.print("Turning HUMIDIFIER: OFF\n");
    //     digitalWrite(HUMIDIFIER, TURN_OFF);
    // }
}


void displayInfo() {

    if (is_daytime()) {
        Serial.print("DAY");
    } else {
        Serial.print("NIGHT");
    }

    if (state.dehumidifier) {
        digitalWrite(RED_LED, HIGH);
        digitalWrite(BLUE_LED, HIGH);
        delay(500);
        digitalWrite(RED_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
        Serial.print(" | DEHUMIDIFIER: ON");
    }

    if (state.arcond) {
        digitalWrite(GREEN_LED, HIGH);
        delay(500);
        digitalWrite(GREEN_LED, LOW);
        Serial.print("| AC: ON");
    }

    Serial.print("\n");
}

void setup() {

    Serial.begin(9600);
    dht.begin();

    pinMode(ARCOND, OUTPUT);
    pinMode(DEHUMIDIFIER, OUTPUT);
    // pinMode(HUMIDIFIER, OUTPUT);
    digitalWrite(ARCOND, TURN_OFF);
    digitalWrite(DEHUMIDIFIER, TURN_OFF);
    // digitalWrite(HUMIDIFIER, TURN_OFF);

    Serial.print("Started!\n");
}

void loop() {

    delay(3000);

    // Temperature
    controlTemperature();
    // delay(500);

    // Humidity
    controlHumidity();

    Serial.print("\n");

    // show stats
    displayInfo();
}
