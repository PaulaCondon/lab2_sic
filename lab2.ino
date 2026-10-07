#include <DHT.h>
#include "model.h"

// ==============================
// PINES
// ==============================

#define DHTPIN D5
#define DHTTYPE DHT11

#define TRIG_PIN D1
#define ECHO_PIN D2

#define LDR_PIN A0

DHT dht(DHTPIN, DHTTYPE);

// Modelo entrenado
Eloquent::ML::Port::RandomForest modelo;


// ==============================
// LEER DISTANCIA
// ==============================

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


// ==============================
// SETUP
// ==============================

void setup() {

  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  dht.begin();

  delay(2000);

  Serial.println();
  Serial.println("===============================");
  Serial.println("      TINY IA - ESP8266");
  Serial.println("===============================");
}


// ==============================
// LOOP
// ==============================

void loop() {

  float distancia = leerDistancia();
  int ldr = analogRead(LDR_PIN);
  float temperatura = dht.readTemperature();
  float humedad = dht.readHumidity();


  // Comprobar sensores
  if (distancia < 0) {
    Serial.println("ERROR: SIN ECO");
    delay(1000);
    return;
  }

  if (isnan(temperatura) || isnan(humedad)) {
    Serial.println("ERROR: DHT11");
    delay(1000);
    return;
  }


  // MUY IMPORTANTE:
  // mismo orden usado durante el entrenamiento
  float datos[] = {
    distancia,
    (float)ldr,
    temperatura,
    humedad
  };


  // IA
  int prediccion = modelo.predict(datos);


  // Mostrar sensores
  Serial.println("--------------------------------");

  Serial.print("Distancia:    ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("Luminosidad:  ");
  Serial.println(ldr);

  Serial.print("Temperatura:  ");
  Serial.print(temperatura);
  Serial.println(" C");

  Serial.print("Humedad:      ");
  Serial.print(humedad);
  Serial.println(" %");

  Serial.println();

  // Resultado
  Serial.print("PREDICCION IA: ");

  if (prediccion == 1) {
    Serial.println("PERSONA PRESENTE");
  }
  else {
    Serial.println("PERSONA AUSENTE");
  }

  delay(1000);
}