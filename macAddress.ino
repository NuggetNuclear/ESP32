#include <WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.println();
  WiFi.disconnect(true);
  delay(100);

  WiFi.mode(WIFI_STA); 
  delay(500); 
  Serial.print("Dirección MAC de este ESP32 (Receptor): ");
  Serial.println(WiFi.macAddress());
}

void loop(){
}