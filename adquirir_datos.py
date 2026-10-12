import serial
import csv
import time

PUERTO = "COM8"
BAUDIOS = 115200
TIEMPO = 60  # segundos por estado

ser = serial.Serial(PUERTO, BAUDIOS, timeout=1)

# El ESP8266 suele reiniciarse al abrir el puerto
time.sleep(2)
ser.reset_input_buffer()

archivo = "sensor_data.csv"

with open(archivo, "w", newline="", encoding="utf-8") as f:

    writer = csv.writer(f)

    # Encabezado
    writer.writerow([
        "distancia",
        "ldr",
        "temp",
        "humedad",
        "etiqueta"
    ])

    # ==========================
    # PERSONA PRESENTE
    # ==========================

    input(
        "Ponete frente al sensor y presioná ENTER para comenzar PRESENTE..."
    )

    ser.write(b"P\n")

    print("\nGrabando PRESENTE durante 60 segundos...\n")

    inicio = time.time()

    while time.time() - inicio < TIEMPO:

        linea = ser.readline().decode(
            "utf-8", errors="ignore"
        ).strip()

        partes = linea.split(",")

        if len(partes) == 5:

            # No guardar errores del ultrasónico
            if partes[0] != "-1.00":

                writer.writerow(partes)

                print(linea)

    # ==========================
    # PERSONA AUSENTE
    # ==========================

    input(
        "\nAhora salí del campo del sensor y presioná ENTER para comenzar AUSENTE..."
    )

    ser.write(b"A\n")

    print("\nGrabando AUSENTE durante 60 segundos...\n")

    inicio = time.time()

    while time.time() - inicio < TIEMPO:

        linea = ser.readline().decode(
            "utf-8", errors="ignore"
        ).strip()

        partes = linea.split(",")

        if len(partes) == 5:

            if partes[0] != "-1.00":

                writer.writerow(partes)

                print(linea)

ser.close()

print("\n================================")
print("CAPTURA TERMINADA")
print("Archivo guardado como:")
print("sensor_data.csv")
print("================================")