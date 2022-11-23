/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp32-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <HCSR04.h>
#include <string.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
const char* ssid = "ROG_Phone3_4601";
const char* password = "carlios123";
String scan_result;

#define OPEN_VALVE_DURATION_S 10
#define TRIG_PIN 22
#define ECHO_PIN 23

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

HCSR04 hc(TRIG_PIN, ECHO_PIN);

void setup() {
  Serial.begin(115200); 
  Serial2.begin(9600);
  Serial2.setTimeout(100);


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
  if(Serial2.available() > 0){
    scan_result = Serial2.readString();
    scan_result.trim();
    scan_result = scan_result.substring(0, 13);
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
    // Serial.println("GET request to " + serverPath);
    if(!scanned){ 
      // Serial.println("Nothing scanned yet");
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

  String message;
  if(json.hasOwnProperty("message")){
    message = (const char*) json["message"];
  }


  // If authenticated
  if(message == "Success"){
    // Open valve
    float water_usage = 50000.0;
    Serial.println("Opening valve");


    // Open valve for 15 seconds
    int timer_start = millis(), timer_end = millis();
    while((timer_end - timer_start) < OPEN_VALVE_DURATION_S * 1000){
      // Listen for QR scan
      if(Serial2.available()){
        scan_result = Serial2.readString();
        scan_result.trim();
        scanned = true;
        scan_result = scan_result.substring(0, 13);
        Serial.println("Scan result = " + scan_result);

        break;
      }

      // Listen for ultrasonic scanner
      if(hc.dist() != 0.00 && hc.dist() < 10.00){
        Serial.println("Filling bottle");
      }
      timer_end = millis();
    }


    // Open valve duration ended
    Serial.println("Valve closed");
    // Send data to database
    HTTPClient http;
    String updateWaterUsagePath = serverName;
    //  + "?qrcode="+ scan_result + "&water_usage=" + String(water_usage);
    Serial.println("Posting on " + updateWaterUsagePath);
    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // String httpRequestData = "{\"qrcode\":\"" + scan_result + "\",\"water_usage\":\"" + String(water_usage) + "\"" + "}";
    String httpRequestData = "qrcode=" + scan_result + "&water_usage=" + String(water_usage);
    Serial.println("Data sent = " + httpRequestData);

    // POSTING
    int httpResponseCode = http.POST(httpRequestData);

    if(httpResponseCode > 0){
      Serial.print("http response code POST: ");
      Serial.println(httpResponseCode);
      
    }
    else{
      Serial.println("WiFi disconnected");
    }
    http.end();
   
  }
  else{ // If not authenticated
      Serial.println("httpresponse code POST: ");
      // Serial.println(httpResponseCode);
      Serial.println("Not authenticated");
      return;
  }
  
  // clear Serial
  Serial2.flush();
  
}