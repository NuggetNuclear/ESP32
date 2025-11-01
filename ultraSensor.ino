#include <esp_now.h>
#include <WiFi.h>
#include "private.h"

// Pines
const int TRIG_PIN = 14; // Trig (OUT)
const int ECHO_PIN = 13; // Echo (IN)
const int LED_PIN = 2;   // Pin del LED integrado

// Variables del sensor
long duration;
float distance_cm = 600.0;
const long TIMEOUT = 50000;

// Tiempos del sensor y LED
unsigned long previousSensorTime = 0;
const long sensorInterval = 200;

unsigned long previousBlinkTime = 0;
int blinkStep = 0;
int currentAlertLevel = 1;
int lastSentAlertLevel = 0;

typedef struct struct_message {
    float distance;
    int   alert_level;
} struct_message;


struct_message ESP_msg;
esp_now_peer_info_t peerInfo;

void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nEstado del último envío: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Éxito" : "Fallo");
}

void initEspNow() {

    if (esp_now_init() != ESP_OK) {
        Serial.println("Error al inicializar ESP-NOW");
        return;
    }

    memcpy(peerInfo.peer_addr, camAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Error al añadir el peer");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
}

void setup() {

    Serial.begin(115200);
    Serial.println("\nIniciando UltraSensor...");

    // Configuración de pines
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);


    digitalWrite(LED_PIN, HIGH);

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_STA);
    delay(100);

    initEspNow();
    digitalWrite(LED_PIN, LOW);

    Serial.println("ESP-NOW Inicializado");

    for(int i = 0; i <= 10; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
}

void measureDistance() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, TIMEOUT);

    if (duration == 0) {
        distance_cm = 500.0;
        Serial.println("Error o fuera de rango.");
    } else {
        distance_cm = (duration * 0.0343) / 2.0;
        Serial.print("Distancia: ");
        Serial.print((int)distance_cm);
        Serial.println(" cm");
    }

    if (distance_cm <= 50.0) { currentAlertLevel = 5; }
    else if (distance_cm <= 100.0) { currentAlertLevel = 4; }
    else if (distance_cm <= 200.0) { currentAlertLevel = 3; }
    else if (distance_cm <= 300.0) { currentAlertLevel = 2; }
    else { currentAlertLevel = 1; }

    if (currentAlertLevel != lastSentAlertLevel) {
        Serial.print("¡Cambio de estado detectado! Enviando Nivel: ");
        Serial.println(currentAlertLevel);

        ESP_msg.distance = distance_cm;
        ESP_msg.alert_level = currentAlertLevel;

        esp_err_t result = esp_now_send(camAddress, (uint8_t *) &ESP_msg, sizeof(ESP_msg));
        
        if (result != ESP_OK) {
          Serial.println("Error al poner mensaje en la cola de envío");
        }

        lastSentAlertLevel = currentAlertLevel;
    }
}

void updateLed(unsigned long currentTime) {
    long timeDiff = currentTime - previousBlinkTime;

    switch (currentAlertLevel) {
    case 1:
        digitalWrite(LED_PIN, LOW);
        blinkStep = 0;
        break;

    case 2: // 1 Hz
        if (blinkStep == 0 && timeDiff >= 900) {
            digitalWrite(LED_PIN, HIGH);
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 100) {
            digitalWrite(LED_PIN, LOW); 
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 3: // 2 Hz
        if (blinkStep == 0 && timeDiff >= 400) {
            digitalWrite(LED_PIN, HIGH);
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 100) {
            digitalWrite(LED_PIN, LOW);
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 4: // 4 Hz
        if (blinkStep == 0 && timeDiff >= 200) {
            digitalWrite(LED_PIN, HIGH);
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 50) {
            digitalWrite(LED_PIN, LOW);
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 5: // Fijo Encendido
        digitalWrite(LED_PIN, HIGH);
        blinkStep = 0;
        break;
    }
}

void loop()
{
    unsigned long currentTime = millis();

    if (currentTime - previousSensorTime >= sensorInterval) {
        previousSensorTime = currentTime;
        measureDistance();
    }

    updateLed(currentTime);
}