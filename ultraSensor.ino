/*
 * Código de Sensor Ultrasónico con 5 Niveles de Alerta
 * - Trig en  PIN D14
 * - Echo en  PIN D13
 * - LED Integrado (D2) para indicar niveles de alerta
 */

// Pines
const int TRIG_PIN = 14; // Trig (OUT)
const int ECHO_PIN = 13; // Echo (IN)
const int LED_PIN = 2;   // Pin del LED integrado

// Variables del sensor
unsigned long duration;
float distance_cm = 600.0;
const unsigned long TIMEOUT = 50000;

// Tiempos del sensor y LED
unsigned long previousSensorTime = 0;
const unsigned long sensorInterval = 60;

unsigned long previousBlinkTime = 0;
int blinkStep = 0;
int currentAlertLevel = 1;

void setup()
{
    Serial.begin(115200);
    Serial.println("Inicio del Sensor");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, LOW);
}

// Medicion de distancia y actualización del nivel de alerta
void measureDistance()
{
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    duration = pulseIn(ECHO_PIN, HIGH, TIMEOUT);

    if (duration == 0)
    {
        distance_cm = 500.0;
        Serial.println("Error o fuera de rango.");
    }
    else
    {
        distance_cm = (duration * 0.0343) / 2.0;
        Serial.print("Distancia: ");
        Serial.print(distance_cm);
        Serial.println(" cm");
    }

    if (distance_cm <= 50.0)
    {
        // 0.2m - 0.5m
        currentAlertLevel = 5;
    }
    else if (distance_cm <= 100.0)
    {
        // 0.5m - 1m
        currentAlertLevel = 4;
    }
    else if (distance_cm <= 200.0)
    {
        // 1m - 2m
        currentAlertLevel = 3;
    }
    else if (distance_cm <= 300.0)
    {
        // 2m - 3m
        currentAlertLevel = 2;
    }
    else
    {
        // 3m +
        currentAlertLevel = 1;
    }
}

// Manejo del LED
void updateLed(unsigned long currentTime)
{

    unsigned long timeDiff = currentTime - previousBlinkTime;

    switch (currentAlertLevel)
    {

    case 1:
        digitalWrite(LED_PIN, LOW);
        blinkStep = 0;
        break;

    case 2: // Nivel 2: 2m - 3m (Parpadeo Lento - 1 Hz)
        // Ciclo total: 1000ms
        if (blinkStep == 0 && timeDiff >= 900)
        {                                // 900ms OFF
            digitalWrite(LED_PIN, HIGH); // Pulso ON
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 100)
        {                               // 100ms ON
            digitalWrite(LED_PIN, LOW); // Pausa larga
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 3: // Nivel 3: 1m - 2m (Parpadeo Medio - 2 Hz)
        // Ciclo total: 500ms
        if (blinkStep == 0 && timeDiff >= 400)
        { // 400ms OFF
            digitalWrite(LED_PIN, HIGH);
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 100)
        { // 100ms ON
            digitalWrite(LED_PIN, LOW);
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 4: // Nivel 4: 0.5m - 1m (Parpadeo Muy Rápido - 4 Hz)
        // Ciclo total: 250ms
        if (blinkStep == 0 && timeDiff >= 200)
        { // 200ms OFF
            digitalWrite(LED_PIN, HIGH);
            previousBlinkTime = currentTime;
            blinkStep = 1;
        }
        else if (blinkStep == 1 && timeDiff >= 50)
        { // 50ms ON (pulso corto)
            digitalWrite(LED_PIN, LOW);
            previousBlinkTime = currentTime;
            blinkStep = 0;
        }
        break;

    case 5: // Nivel 5: 0.2m - 0.5m (Encendido fijo)
        digitalWrite(LED_PIN, HIGH);
        blinkStep = 0; // Resetear el parpadeo
        break;
    }
}

void loop()
{
    unsigned long currentTime = millis();

    if (currentTime - previousSensorTime >= sensorInterval)
    {
        previousSensorTime = currentTime;
        measureDistance();
    }
    updateLed(currentTime);
}