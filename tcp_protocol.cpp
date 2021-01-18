#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "higrow_sensors.h"
#include <StreamUtils.h>
#include "tcp_protocol.h"



TcpClient::TcpClient(IPAddress ip){
  _ip = ip;
}

void TcpClient::end(){
  _client.println("end");
  delay(100);
  _client.stop();
}

void TcpClient::begin(){
    if (!_client.connect(_ip, PORT)){
      Serial.println("Couldn't connect to the host");
      return;
    }
  }

StaticJsonDocument<200> TcpClient::ConvertToJson(SensorData s){
  //this shortens up the code for the convertion between struct and json
  StaticJsonDocument<200> d; //I guess 200 bytes is more than enough
  d["temp"]=s.temp;
  d["humidity"] = s.hum;
  d["soil_moisture"] = s.soil_moisture;
  d["soil_salt"] = s.soil_salt;
  d["lux"] = s.lux;
  d["timestamp"] = s.timestamp;
  d["mac"] = WiFi.macAddress(); // the identifier of every device
  return d;
  
  }

void TcpClient::SendData(SensorData s){
    StaticJsonDocument<200> d = TcpClient::ConvertToJson(s);
    WriteBufferingStream buff{_client, 200};
    serializeJson(d, buff); //print it out to serial for testing
    buff.flush();
  }

void TcpClient::GetDateFromServer(){
  StaticJsonDocument<200> d;
  d["request"] = "date";
  WriteBufferingStream buff(_client, 200);
  serializeJson(d, buff);
  buff.flush(); 
  //Now wait for the packet to arrive
  StaticJsonDocument<200> res  = TcpClient::GetResponse();
  int date = res["date"];
  if (date == 0){
    return; //don't set the date if the server hasn't answered 
  }
  struct timeval sdate;
  sdate.tv_sec = date;
  settimeofday(&sdate, NULL); 
}
StaticJsonDocument<200> TcpClient::GetResponse(){
  int t = 0;
  while (_client.available() == 0){
    t++;
    if (t >= TIMEOUT){
      Serial.println("The server hasn't awnsered, aborting getting date from server...");
      _client.stop();
      StaticJsonDocument<100> err;
      err["error"] = "error";
      return err;
    }      

    delay(500);  
  }
  String response = _client.readString();
  StaticJsonDocument<200> res;
  DeserializationError err = deserializeJson(res, response);
  if (err){
    Serial.println("Couldn't parse the json..");
    Serial.println(err.c_str());  
  }
  return res;
}

void TcpClient::GetSleepTime(int timestosend, int * sleep_time){
  StaticJsonDocument<200> d;
  d["request"] = "time_reconnect";
  WriteBufferingStream buff(_client, 200);
  serializeJson(d, buff);
  buff.flush(); 
  //Now wait for the packet to arrive
  StaticJsonDocument<200> res  = TcpClient::GetResponse();
  int reconnect_time = res["time_reconnect"];
  Serial.printf("%d reconnect_time\n", reconnect_time);
  if (reconnect_time == 0){
    return; //aka maintain the default sleep time  
  }
  //TODO: calculate sleep time with times to send and reconnect_time (this applies only on nodes connected to an hybrid node)
  }
