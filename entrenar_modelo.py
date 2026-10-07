import pandas as pd

from sklearn.ensemble import RandomForestClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, confusion_matrix

from micromlgen import port


# ==========================================
# 1. CARGAR LOS DATOS
# ==========================================

data = pd.read_csv("sensor_data.csv")

print("Cantidad total de datos:", len(data))

print("\nCantidad por estado:")
print(data["etiqueta"].value_counts())


# ==========================================
# 2. VARIABLES DE ENTRADA Y SALIDA
# ==========================================

# Entradas del modelo
X = data[
    ["distancia", "ldr", "temp", "humedad"]
]

# Salida:
# presente = 1
# ausente = 0
y = (
    data["etiqueta"] == "presente"
).astype(int)


# ==========================================
# 3. DIVIDIR DATOS
# ==========================================

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.20,
    random_state=42,
    stratify=y
)


# ==========================================
# 4. CREAR Y ENTRENAR MODELO
# ==========================================

modelo = RandomForestClassifier(
    n_estimators=10,
    max_depth=5,
    random_state=42
)

modelo.fit(X_train, y_train)


# ==========================================
# 5. EVALUAR MODELO
# ==========================================

predicciones = modelo.predict(X_test)

exactitud = accuracy_score(
    y_test,
    predicciones
)

print("\n==============================")
print(" RESULTADOS DEL MODELO")
print("==============================")

print(
    "Exactitud:",
    round(exactitud * 100, 2),
    "%"
)

print("\nMatriz de confusion:")
print(
    confusion_matrix(
        y_test,
        predicciones
    )
)


# ==========================================
# 6. GENERAR MODELO PARA EL ESP8266
# ==========================================

codigo_cpp = port(modelo)

with open(
    "model.h",
    "w",
    encoding="utf-8"
) as archivo:

    archivo.write(codigo_cpp)


print("\n==============================")
print("Archivo model.h generado")
print("==============================")