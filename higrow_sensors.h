#ifndef HIGROW_SENSORS_H
#define HIGROW_SENSORS_H
#include <DHT12.h>
#include <BH1750.h>
#include <sys/time.h>
#include <Wire.h>

const int I2C_SDA=25;
const int I2C_SCL=26;
const int DHT12_PIN=16;
const int BAT_ADC=33;
const int SALT_PIN=34;
const int SOIL_PIN=32;
const int BOOT_PIN=0;
const int POWER_CTRL=4;
const int DS18B20_PIN=21;    

typedef struct SensorData{
  float temp;
  uint16_t hum;
  uint16_t soil_moisture;
  uint16_t soil_salt;
  float lux;
  int timestamp;
  
};

class HiGrowSensors
{
  public:
    HiGrowSensors();
    void toggleSensors();
    void begin();
    SensorData measure_sensors();
  private:
    DHT12* d1;
    BH1750 lightMeter;
    bool TOGGLE_SENSORS = false;
};
#endif
