#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "model.h"
#include "red_neuronal.h"

// =====================================
// CONFIGURACION WIFI
// =====================================

const char* ssid = "Red";
const char* password = "Contraseña";

// IP de la computadora con servidor.py
const char* servidor =
  "http://192.168.1.6:5000/presencia";

// =====================================
// CONFIGURACION SENSORES
// =====================================

#define DHTPIN D5
#define DHTTYPE DHT11

#define TRIG_PIN D1
#define ECHO_PIN D2
#define LDR_PIN A0

// LED que representa el ventilador
#define LED_PIN D6

DHT dht(DHTPIN, DHTTYPE);

// =====================================
// MODELOS DE INTELIGENCIA ARTIFICIAL
// =====================================

// Modelo original Random Forest
Eloquent::ML::Port::RandomForest modelo;

// Red neuronal entrenada en Python
TinyIA::RedNeuronal redNeuronal;

// =====================================
// CONFIGURACION GENERAL
// =====================================

// Intervalo de 30 segundos
const unsigned long INTERVALO = 30000;

// Umbral de temperatura
const float UMBRAL_TEMPERATURA = 28.0;

// Estado del ventilador
bool ventiladorEncendido = false;

// Contador de mediciones
unsigned long numeroMedicion = 0;

// =====================================
// CONEXION WIFI
// =====================================

bool conectarWiFi() {

  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  Serial.println("Conectando al WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - inicio < 15000) {

    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WIFI CONECTADO");

    Serial.print("IP ESP8266: ");
    Serial.println(WiFi.localIP());

    return true;
  }

  Serial.println("ERROR: WiFi no conectado");

  return false;
}

// =====================================
// LIGHT SLEEP
// =====================================

void configurarLightSleep() {

  if (WiFi.setSleepMode(WIFI_LIGHT_SLEEP)) {

    Serial.println("LIGHT SLEEP HABILITADO");

  } else {

    Serial.println("ERROR AL HABILITAR LIGHT SLEEP");
  }
}

// =====================================
// ENVIAR ESTADO HTTP
// =====================================

void enviarNotificacion(float temperatura,
                        float distancia,
                        bool ventilador,
                        bool presencia) {

  if (!conectarWiFi()) {

    Serial.println("Servidor: SIN WIFI");
    return;
  }

  WiFiClient cliente;
  HTTPClient http;

  if (!http.begin(cliente, servidor)) {

    Serial.println("Servidor: ERROR HTTP");
    return;
  }

  http.setTimeout(5000);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String mensaje = "{";

  mensaje += "\"estado\":\"";
  mensaje += presencia ? "PRESENTE" : "AUSENTE";
  mensaje += "\",";

  mensaje += "\"temperatura\":";
  mensaje += String(temperatura, 2);
  mensaje += ",";

  mensaje += "\"distancia\":";
  mensaje += String(distancia, 2);
  mensaje += ",";

  mensaje += "\"ventilador\":\"";
  mensaje += ventilador ? "ON" : "OFF";
  mensaje += "\"";

  mensaje += "}";

  int codigo = http.POST(mensaje);

  if (codigo == 200) {

    Serial.print("Servidor: OK - estado enviado: ");

    if (presencia) {
      Serial.println("PRESENTE");
    } else {
      Serial.println("AUSENTE");
    }

  } else {

    Serial.print("Servidor: ERROR ");
    Serial.println(codigo);

    if (codigo < 0) {
      Serial.println(http.errorToString(codigo));
    }
  }

  http.end();
}

// =====================================
// LEER SENSOR ULTRASONICO
// =====================================

float leerDistancia() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  unsigned long duracion =
    pulseIn(ECHO_PIN, HIGH, 30000);

  if (duracion == 0) {
    return -1;
  }

  return duracion * 0.0343 / 2.0;
}

// =====================================
// SETUP
// =====================================

void setup() {

  Serial.begin(115200);
  delay(1000);

  // Sensor ultrasonico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Sensor DHT11
  dht.begin();

  Serial.println();
  Serial.println("==============================");
  Serial.println("       TINY IA - ESP8266");
  Serial.println("==============================");

  Serial.println("Random Forest + Red Neuronal");

  conectarWiFi();

  configurarLightSleep();
}

// =====================================
// LOOP PRINCIPAL
// =====================================

void loop() {

  numeroMedicion++;

  Serial.println();
  Serial.println("==============================");

  Serial.print("MEDICION NUMERO: ");
  Serial.println(numeroMedicion);

  Serial.println("==============================");

  // ===================================
  // 1. ADQUISICION DE DATOS
  // ===================================

  float distancia = leerDistancia();

  int ldr = analogRead(LDR_PIN);

  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();

  // ===================================
  // 2. VERIFICACION DE SENSORES
  // ===================================

  if (distancia < 0 ||
      isnan(temperatura) ||
      isnan(humedad)) {

    Serial.println("ERROR EN SENSORES");

    digitalWrite(LED_PIN, LOW);
    ventiladorEncendido = false;

    delay(INTERVALO);
    return;
  }

  // ===================================
  // 3. MOSTRAR SENSORES
  // ===================================

  Serial.println();
  Serial.println("DATOS DE SENSORES:");

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("LDR: ");
  Serial.println(ldr);

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Humedad: ");
  Serial.print(humedad);
  Serial.println(" %");

  // ===================================
  // 4. PREPARAR DATOS IA
  // ===================================

  float datos[] = {
    distancia,
    (float)ldr,
    temperatura,
    humedad
  };

  // ===================================
  // 5. RANDOM FOREST
  // ===================================

  int prediccionRF = modelo.predict(datos);

  bool presencia = (prediccionRF == 1);

  // ===================================
  // 6. RED NEURONAL
  // ===================================

  int prediccionRed = redNeuronal.predict(datos);

  // ===================================
  // 7. COMPARACION DE MODELOS
  // ===================================

  Serial.println();
  Serial.println("COMPARACION DE MODELOS:");

  Serial.print("Random Forest: ");

  if (prediccionRF == 1) {
    Serial.println("PERSONA PRESENTE");
  } else {
    Serial.println("PERSONA AUSENTE");
  }

  Serial.print("Red Neuronal: ");

  if (prediccionRed == 1) {
    Serial.println("PERSONA PRESENTE");
  } else {
    Serial.println("PERSONA AUSENTE");
  }

  if (prediccionRF == prediccionRed) {

    Serial.println("RESULTADO: COINCIDEN");

  } else {

    Serial.println("RESULTADO: NO COINCIDEN");
  }

  // ===================================
  // 8. FUSION SENSORIAL
  // ===================================

  ventiladorEncendido =
    presencia &&
    (temperatura > UMBRAL_TEMPERATURA);

  digitalWrite(
    LED_PIN,
    ventiladorEncendido ? HIGH : LOW
  );

  Serial.println();
  Serial.println("FUSION SENSORIAL:");

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Umbral: ");
  Serial.print(UMBRAL_TEMPERATURA);
  Serial.println(" C");

  Serial.print("VENTILADOR: ");

  if (ventiladorEncendido) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }

  // ===================================
  // 9. COMUNICACION HTTP
  // ===================================

  enviarNotificacion(
    temperatura,
    distancia,
    ventiladorEncendido,
    presencia
  );

  // ===================================
  // 10. INTERVALO DE BAJO CONSUMO
  // ===================================

  Serial.println();
  Serial.println("Intervalo de inactividad: 30 segundos");
  Serial.println("Light Sleep automatico habilitado");

  delay(INTERVALO);
}
