#include <Arduino.h>
#include "EEPROM.h"

struct Sensor {
  uint32_t ledPin;
  uint32_t resistorPin;
};

struct Result {
  uint32_t pin;
  float volume;
  int result;
};

const int sensors_count = 2;
Sensor sensors[sensors_count] = {{10, 20}, {5, 10}};

void recalibrate() {
  int eeprom_idx = 0;
  char buf[255];
  int sample_amount;

  const int max_samples = (int)(((EEPROM.length() - (EEPROM.length()%sizeof(Result)))/sizeof(Result)/sensors_count));

  do {
    sprintf(buf, "Sample amount (max %d): ", max_samples);
    Serial.print(buf);
    if(Serial.available() > 0) {
      Serial.readBytesUntil('\n', buf, 255);
      sample_amount = atoi(buf);
      if(sample_amount > max_samples) {
        Serial.println("ERROR: Sample amount above allowable memory limit.");
      }
    }
  } while (sample_amount > max_samples);

  for(int i = 0; i < max_samples; ++i) {
    Serial.print("Volume (ml): ");
    if (Serial.available() > 0) {
      Serial.readBytesUntil('\n', buf, 255);
      int vol = atoi(buf);
      Serial.println(vol);

      for(int i = 0; i < sensors_count; ++i) {
        digitalWrite(sensors[i].ledPin, HIGH);
        delay(10);

        int result = analogRead(sensors[i].resistorPin);
        digitalWrite(sensors[i].ledPin, LOW);
        delay(10);

        sprintf(buf, "Volume: %d | Sensor: %d | Result: %d\n", vol, i, result);
        Serial.print(buf);

        Result res{sensors[i].resistorPin, vol, result};
        EEPROM.put(eeprom_idx, &res);
        eeprom_idx += sizeof(res);
      }
    }
  }
}

void print_results() {
  char buf[255];

  for(int i = 0; i < EEPROM.length(); i += sizeof(Result)) {
    Result res{};
    EEPROM.get(i, res);

    sprintf(buf, "Volume: %d | Sensor: %d | Result: %d\n", res.volume, res.pin, res.result);
    Serial.print(buf);
  }
}

void setup() {
  Serial.begin(9600);
  for(Sensor sensor : sensors) {
    pinMode(sensor.ledPin, OUTPUT);
    pinMode(sensor.resistorPin, INPUT);
  }

  while(true) {
    Serial.println("Recalibrate? Y/n");
    if (Serial.available() > 0) {
      char rx_byte = Serial.read();
      if(rx_byte == 'Y' || rx_byte == 'y') {
        recalibrate();
        break;
      } else if (rx_byte == 'n' || rx_byte == 'N') {
        break;
      }
    }
  }

  print_results();
}

void loop() {

}