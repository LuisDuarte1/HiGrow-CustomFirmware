#include <Arduino.h>
#include <DHT12.h>
#include <BH1750.h>
#include <Wire.h>
#include <sys/time.h>
#include "higrow_sensors.h"




void HiGrowSensors::toggleSensors(){
  if(TOGGLE_SENSORS == false){
    TOGGLE_SENSORS = true;
    pinMode(POWER_CTRL, OUTPUT);
    digitalWrite(POWER_CTRL, 1);
 
  }  else {
    TOGGLE_SENSORS = false;
    pinMode(POWER_CTRL, OUTPUT);
    digitalWrite(POWER_CTRL, 0);
  }
}

HiGrowSensors::HiGrowSensors(){
  d1 = new DHT12(DHT12_PIN, true);
  }

void HiGrowSensors::begin(){
  HiGrowSensors::toggleSensors();
  delay(1000);
  Wire.begin(I2C_SDA, I2C_SCL);
  lightMeter.begin();
  d1->begin();
  delay(500);
  }

SensorData HiGrowSensors::measure_sensors(){
  //this part of the code implements the dht12 stuff
  float temp = d1->readTemperature();
  float hum = d1->readHumidity();
  //this part implemets the humidity of the soil using a linear function which technically it isn't but it serves for this purpose
  float soil_moisture = map(analogRead(SOIL_PIN), 0, 4095, 100, 0); //this needs more calibration?
  float soil_salt = map(analogRead(SALT_PIN), 0, 4095, 0, 100);
  float lux = lightMeter.readLightLevel();
  lux = lightMeter.readLightLevel();
  delay(500);
  lux = lightMeter.readLightLevel();
  
  Serial.printf("temperatura: %.f ºC\n", temp);
  Serial.printf("humidade: %.f %\n", hum);
  Serial.printf("Humidade do Solo %.f\n", soil_moisture);
  Serial.printf("Sal do Solo %.f\n", soil_salt);
  Serial.printf("Luminosidade %.f\n", lux); 
  
  SensorData s;
  s.temp = temp;
  s.hum = hum;
  s.soil_moisture = soil_moisture;
  s.soil_salt = soil_salt;
  s.lux = lux;
  struct timeval tempo;
  gettimeofday(&tempo, NULL);
  s.timestamp = tempo.tv_sec;
  return s;
}
