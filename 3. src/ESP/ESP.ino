/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/



#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
const char* ssid = "ROG_Phone3_4601";
const char* password = "carlios123";

//Your Domain name with URL path or IP address with path
String serverName = "https://qr-dispenser.herokuapp.com/api/drink";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
bool scanned = false;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

void setup() {
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {

  // scan QR
  String scan_result; 
  if(Serial.available()){
    scan_result = Serial.readString();
    scan_result.trim();
    scanned = true;
    Serial.println("Scan result = " + scan_result);

  }

  // ======================================================================================================================================
  // Check authentication
  JSONVar json;
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    String serverPath = serverName + "/" + scan_result;
    // String serverPath = serverName + "/1332102958524";
    if(!scanned){ 
      Serial.println("Nothing scanned yet");
      return;
    }
    scanned = false;
    http.begin(serverPath.c_str());
    
    int httpResponseCode = http.GET();
      
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);

      // parse json data
      json = JSON.parse(payload);

      if(JSON.typeof(json) == "undefined"){
        Serial.println("Parsing failed!");
        return;
      }

      Serial.print("JSON object = ");
      Serial.println(json);
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }

    http.end();
    
  }
  else {
      Serial.println("WiFi Disconnected");
  }


  if(json.hasOwnProperty("success")){
    Serial.println((const char*) json["success"]);
  }
  
  // If autheticated
  // if(json["message"] == "success"){
  //   // open valve
  //   Serial.println("Opening valve");
  //   int timer_start = millis(), timer_end = millis();
  //   if((timer_end - timer_start) < 15000){
  //     Serial.println("Listening for QR");
  //     Serial.println("Valve opened");
  //   // if(Serial2.readable()){
  //   //   return;
  //   // }

    
  //    timer_end = millis();
  //   }
  //   else{
  //     Serial.println("Valve closed");

  //     // send data to database
  //   }

  //   // Listen for QR scan


  // }
  // else{
  //   Serial.println("Invalid QR Code");
  // }
  
}