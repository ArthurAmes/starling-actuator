#include <Arduino.h>
#include "EEPROM.h"

struct Sensor {
  uint32_t ledPin;
  uint32_t resistorPin;
};

struct Result {
  uint32_t pin;
  float volume;
  float voltage;
  int result;
};

const int sensors_count = 2;
Sensor sensors[sensors_count] = {{17, 14}, {16, 15}};

void read_bytes_until_blocking(char delimeter, char* buf, int max) {
  while(Serial.available() <= 0) {}

  Serial.readBytesUntil('\n', buf, 255);
}

void recalibrate() {
  int eeprom_idx = 0;
  char buf[255];
  int sample_amount = 0;

  const int max_samples = (int)(((EEPROM.length() - (EEPROM.length()%sizeof(Result)))/sizeof(Result)/sensors_count))-1;

  do {
    sprintf(buf, "Sample amount (max %d): ", max_samples);
    Serial.print(buf);

    read_bytes_until_blocking('\n', buf, 255);
    sample_amount = atoi(buf);
    if(sample_amount > max_samples) {
      Serial.println("ERROR: Sample amount above allowable memory limit.");
    }
  } while (sample_amount > max_samples);

  Serial.println(sample_amount);

  for(int i = 0; i < sample_amount; ++i) {
    Serial.print("Volume (ml): ");
    read_bytes_until_blocking('\n', buf, 255);
    float vol = atof(buf);
    Serial.println(vol);

    Serial.print("Base Voltage: ");
    read_bytes_until_blocking('\n', buf, 255);
    float vtge = atof(buf);
    Serial.println(vtge);

    for(int j = 0; j < sensors_count; ++j) {
      digitalWrite(sensors[i].ledPin, HIGH);
      delay(10);

      int result = analogRead(sensors[j].resistorPin);
      digitalWrite(sensors[j].ledPin, LOW);
      delay(10);

      Result res{sensors[j].resistorPin, vol, vtge, result};
      EEPROM.put(eeprom_idx, res);
      EEPROM.commit();

      sprintf(buf, "Volume: %.2f | Sensor: %d | Result: %d | Voltage: %.2f\n", res.volume, res.pin, res.result, res.voltage);
      Serial.print(buf);

      eeprom_idx += sizeof(Result);
    }
  }
  Result paddingRes{0xDEADBEEF, 0, 0};
  EEPROM.put(eeprom_idx, paddingRes);
  EEPROM.commit();
}

void print_results() {
  char buf[255];

  for(int i = 0; i < EEPROM.length(); i += sizeof(Result)) {
    Result res;
    EEPROM.get(i, res);

    if(res.pin == 0xDEADBEEF) {
      break;
    }

    sprintf(buf, "Volume: %.2f | Sensor: %d | Result: %d | Voltage: %.2f\n", res.volume, res.pin, res.result, res.voltage);
    Serial.print(buf);
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(1024); // Remember to go back into lib code and fix for actual board.
  for(Sensor sensor : sensors) {
    pinMode(sensor.ledPin, OUTPUT);
    pinMode(sensor.resistorPin, INPUT);
  }

  Serial.println("Recalibrate? Y/n");
  while(true) {
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

  Serial.println("Printing Results: \n");

  print_results();
}

void loop() {

}