#include <WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
  Serial.print("Dirección MAC de este ESP32 (Receptor): ");
  Serial.println(WiFi.macAddress());
}

void loop(){
}