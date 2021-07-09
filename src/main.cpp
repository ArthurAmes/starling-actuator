#include <Arduino.h>
#include "EEPROM.h"

struct Result {
  float volume;
  float voltage;
  int result;
};

const int sample_count = 5;

const int resistor_pin = 13;

char buf[255];

void read_bytes_until_blocking(char delimeter, char* buf, int max) {
  while(Serial.available() <= 0) {}

  Serial.readBytesUntil('\n', buf, 255);
}

Result interpolate_measurement(int measurement) {
  int closest_above_delta = 100000;
  int closest_below_delta = 100000;
  Result closest_above_result;
  Result closest_below_result;

  for(int i = 0; i < EEPROM.length(); i += sizeof(Result)) {
    Result res;
    EEPROM.get(i, res);

    if(res.result == 0xDEAD) {
      break;
    }

    if((res.result > measurement) && (res.result - measurement) < closest_above_delta) {
      closest_above_delta = (res.result - measurement);
      closest_above_result = res;
    }

    if((res.result < measurement) && (measurement - res.result) < closest_below_delta) {
      closest_below_delta = (measurement - res.result);
      closest_below_result = res;
    }
  }

  Result ret;
  ret.result = measurement;
  ret.voltage = (((measurement - closest_below_result.result) / (closest_above_result.result - closest_below_result.result)) * (closest_above_result.voltage - closest_below_result.voltage)) + closest_below_result.voltage;
  ret.volume = (((measurement - closest_below_result.result) / (closest_above_result.result - closest_below_result.result)) * (closest_above_result.volume - closest_below_result.volume)) + closest_below_result.volume;
  return ret;
}

void recalibrate() {
  int eeprom_idx = 0;
  int sample_amount = 0;

  const int max_samples = (int)((EEPROM.length() - (EEPROM.length()%sizeof(Result)))/sizeof(Result))-1;

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

    int result = 0;
    for(int j = 0; j < sample_count; j++) {
      result += analogRead(resistor_pin);
      delay(10);
    }
    result = result / sample_count;

    Result res{vol, vtge, result};
    EEPROM.put(eeprom_idx, res);
    EEPROM.commit();

    sprintf(buf, "Volume: %.2f | Result: %d | Voltage: %.2f\n", res.volume, res.result, res.voltage);
    Serial.print(buf);

    eeprom_idx += sizeof(Result);
  }

  Result paddingRes{0, 0, 0xDEAD};
  EEPROM.put(eeprom_idx, paddingRes);
  EEPROM.commit();
}

void print_results() {
  for(int i = 0; i < EEPROM.length(); i += sizeof(Result)) {
    Result res;
    EEPROM.get(i, res);

    if(res.result == 0xDEAD) {
      break;
    }

    sprintf(buf, "Volume: %.2f | Result: %d | Voltage: %.2f\n", res.volume, res.result, res.voltage);
    Serial.print(buf);
  }
}

void setup() {
  Serial.begin(9600);
  EEPROM.begin(1024); // Remember to go back into lib code and fix for actual board.
  pinMode(resistor_pin, INPUT);

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

}

void loop() {
  int result = 0;
  for(int j = 0; j < sample_count; j++) {
    result += analogRead(resistor_pin);
    delay(10);
  }
  result = result / sample_count;

  Result estimate = interpolate_measurement(result);
  sprintf(buf, "Closest Volume Estimate: %.2f\n", estimate.volume);
  Serial.print(buf);
}