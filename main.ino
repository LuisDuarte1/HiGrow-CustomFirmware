#include <Arduino.h>
#include "time.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <SPIFFS.h>
#include "higrow_sensors.h"
#include "tcp_protocol.h"

const int SECTOMICROSEC = 1000000;
const int TIMESTOSEND  = 5;



RTC_DATA_ATTR int boot_times = 0; //make it use the rtc ram for deep sleep

RTC_DATA_ATTR int next_timestamp_secs = 0;
RTC_DATA_ATTR int next_timestamp_usecs = 0;
struct timeval tv_now;

RTC_DATA_ATTR SensorData sdata[TIMESTOSEND];
RTC_DATA_ATTR int sdata_free = 0; //store the index that the sensordata array is free


RTC_DATA_ATTR char* wifi_ssid = "MEO-72F373";
RTC_DATA_ATTR char* wifi_pass = "29C7C64D84";

HiGrowSensors h1;
TcpClient c(IPAddress(192,168,1,84));

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

RTC_DATA_ATTR int sleep_time = 10;

bool config_enable = false;


void enterDeepSleep(int secs){
  h1.toggleSensors(); //turn off the sensors
  esp_sleep_enable_timer_wakeup(secs * SECTOMICROSEC);
  gettimeofday(&tv_now, NULL);
  next_timestamp_secs = tv_now.tv_sec + secs;
  next_timestamp_usecs = tv_now.tv_usec;
  esp_deep_sleep_start();
}

void get_current_date_ntp(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct timeval set_time;
  gettimeofday(&set_time, NULL);
  while (set_time.tv_sec < 100){
    Serial.println("Couldn't set the time... trying again");
    delay(500);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    gettimeofday(&set_time, NULL);
  }
  settimeofday(&set_time, NULL);
}

void IRAM_ATTR config_button(){
  detachInterrupt(35); //lock the interrupt to one use only
  config_enable = true;
}

void setup(){
  h1.begin();
  Serial.begin(115200);
  boot_times++;
  if(boot_times != 1){
    struct timeval time_Set;
    time_Set.tv_sec = next_timestamp_secs;
    time_Set.tv_usec = next_timestamp_usecs;
    settimeofday(&time_Set, NULL);
  } else if (boot_times == 1) {
    Serial.println("Waiting for button to be pressed");
    pinMode(35, INPUT_PULLUP);
    attachInterrupt(35,config_button, CHANGE);
    delay(2000); //wait for button to be pressed
    detachInterrupt(35);
    if(config_enable == true){
      while (true){
        Serial.println("Config enabled");
        delay(500);
      }
    }
    Serial.print("Connecting to wifi...\n");
    WiFi.enableSTA(true);
    WiFi.begin(wifi_ssid, wifi_pass);
    while(WiFi.status() != WL_CONNECTED){delay(500);} //wait until wifi has connected
    Serial.print("Getting new date...\n");
    delay(500);
    //get_current_date_ntp();
    c.begin();
    c.GetDateFromServer();
    c.GetSleepTime(TIMESTOSEND, &sleep_time); 
    c.end();
  }
  SensorData s = h1.measure_sensors();
  sdata[sdata_free] = s;
  sdata_free++;
  bool transmit_data = (boot_times % TIMESTOSEND) == 0;
  if (transmit_data == true){
    Serial.println("Connecting to wifi...");
    WiFi.enableSTA(true);
    WiFi.begin(wifi_ssid, wifi_pass);
    while(WiFi.status() != WL_CONNECTED){delay(500);} //wait until wifi has connected
    Serial.print("Getting new date...\n");
    //get_current_date_ntp();
    Serial.println("Connecting to server...");
    c.begin(); //connect to server
    c.GetDateFromServer();
    c.GetSleepTime(TIMESTOSEND, &sleep_time);
    for (int i = 0; i<TIMESTOSEND; i++){ //send data one by one
      c.SendData(sdata[i]);
      delay(100);
    }
    c.end();
    sdata_free = 0; //put the index to zero to allow for overriding of sensor data, it's okay because previous data has already been sent
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.printf("The sensor has waken up %d times\n", boot_times);
  struct timeval get_time;
  gettimeofday(&get_time, NULL);
  Serial.println(get_time.tv_sec);
  enterDeepSleep(sleep_time);

  
}


void loop(){
  //this never gets called due to the deep sleep
}
