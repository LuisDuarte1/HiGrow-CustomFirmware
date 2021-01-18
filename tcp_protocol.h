#ifndef TCP_PROTOCOL_H
#define TCP_PROTOCOL_H

#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include "higrow_sensors.h"

/*
 * This protocol is strictly request-reply, the client must request or send something to the server first. However, there isn't any reply when the client sends the sensor data.
 */

const int PORT = 36513;
const int TIMEOUT = 5; //it waits 10 times * time interval

class TcpClient
{
  public:
    TcpClient(IPAddress ip);
    void SendData(SensorData s);
    void begin();
    void end();
    void GetDateFromServer();
    void GetSleepTime(int timestosend, int * sleep_time);
  private:
    WiFiClient _client;
    StaticJsonDocument<200> ConvertToJson(SensorData s);
    IPAddress _ip;
    StaticJsonDocument<200> GetResponse();
};

#endif
