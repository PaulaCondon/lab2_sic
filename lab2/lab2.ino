
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
  "http://192.168.1.12:5000/presencia";

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

// Nueva red neuronal entrenada en Python
TinyIA::RedNeuronal redNeuronal;

// =====================================
// CONFIGURACION GENERAL
// =====================================

// Espera entre mediciones
const unsigned long INTERVALO = 3000;

// Temperatura para activar el LED
const float UMBRAL_TEMPERATURA = 22.0;

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

  Serial.println();
  Serial.println("Conectando al WiFi...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long inicio = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - inicio < 15000) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("WIFI CONECTADO");

    Serial.print("IP del ESP8266: ");
    Serial.println(WiFi.localIP());

    return true;
  }

  Serial.println("ERROR: WiFi no conectado");

  return false;
}


// =====================================
// ENVIAR NOTIFICACION HTTP
// =====================================

void enviarNotificacion(float temperatura,
                        float distancia,
                        bool ventilador) {

  if (!conectarWiFi()) {

    Serial.println("No se puede enviar: sin WiFi");
    return;
  }

  WiFiClient cliente;
  HTTPClient http;

  if (!http.begin(cliente, servidor)) {

    Serial.println("ERROR AL INICIAR HTTP");
    return;
  }

  http.setTimeout(5000);

  http.addHeader(
    "Content-Type",
    "application/json"
  );

  String mensaje = "{";

  mensaje += "\"estado\":\"PRESENTE\",";

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

  Serial.println();
  Serial.println("Enviando notificacion...");
  Serial.println(mensaje);

  int codigo = http.POST(mensaje);

  Serial.print("Codigo HTTP: ");
  Serial.println(codigo);

  if (codigo == 200) {

    Serial.println(
      "NOTIFICACION ENVIADA CORRECTAMENTE"
    );

    Serial.println(http.getString());

  } else {

    Serial.println("ERROR EN NOTIFICACION");

    if (codigo < 0) {
      Serial.println(http.errorToString(codigo));
    } else {
      Serial.println(http.getString());
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

  Serial.println("Sistema iniciado");
  Serial.println("Random Forest + Red Neuronal");
  Serial.println("Deep Sleep retirado");

  conectarWiFi();
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

    // Apagar LED por seguridad
    digitalWrite(LED_PIN, LOW);
    ventiladorEncendido = false;

    delay(INTERVALO);
    return;
  }


  // ===================================
  // 3. MOSTRAR DATOS DE SENSORES
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
  // 4. PREPARAR DATOS PARA IA
  // ===================================

  float datos[] = {
    distancia,
    (float)ldr,
    temperatura,
    humedad
  };


  // ===================================
  // 5. PREDICCION RANDOM FOREST
  // ===================================

  int prediccionRF = modelo.predict(datos);

  bool presencia = (prediccionRF == 1);


  // ===================================
  // 6. PREDICCION RED NEURONAL
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
  // 8. FUSION SENSORIAL AVANZADA
  // ===================================

  // El Random Forest sigue controlando
  // el funcionamiento principal.

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

  if (presencia) {

    enviarNotificacion(
      temperatura,
      distancia,
      ventiladorEncendido
    );

  } else {

    Serial.println();
    Serial.println("No se envia notificacion");
  }


  // ===================================
  // 10. ESPERA ENTRE MEDICIONES
  // ===================================

  Serial.println();
  Serial.println("Esperando siguiente medicion...");

  delay(INTERVALO);
}
