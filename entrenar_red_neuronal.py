
import numpy as np
import pandas as pd

from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from sklearn.neural_network import MLPClassifier
from sklearn.metrics import accuracy_score, confusion_matrix


# ==========================================
# 1. CARGAR LOS DATOS
# ==========================================

datos = pd.read_csv("sensor_data.csv")

print("Cantidad total de datos:", len(datos))

print("\nCantidad por estado:")
print(datos["etiqueta"].value_counts())


# ==========================================
# 2. VARIABLES DE ENTRADA Y SALIDA
# ==========================================

# Entradas de la red neuronal
X = datos[
    ["distancia", "ldr", "temp", "humedad"]
].to_numpy(dtype=float)

# Salida:
# presente = 1
# ausente = 0

y = (
    datos["etiqueta"].str.strip().str.lower()
    == "presente"
).astype(int).to_numpy()


# ==========================================
# 3. DIVIDIR LOS DATOS
# ==========================================

X_train, X_test, y_train, y_test = train_test_split(
    X,
    y,
    test_size=0.20,
    random_state=42,
    stratify=y
)


# ==========================================
# 4. NORMALIZAR LOS DATOS
# ==========================================

escalador = StandardScaler()

X_train_norm = escalador.fit_transform(X_train)

X_test_norm = escalador.transform(X_test)


# ==========================================
# 5. CREAR Y ENTRENAR RED NEURONAL
# ==========================================

red = MLPClassifier(
    hidden_layer_sizes=(6,),
    activation="tanh",
    solver="lbfgs",
    alpha=0.01,
    max_iter=2000,
    random_state=42
)

red.fit(X_train_norm, y_train)


# ==========================================
# 6. EVALUAR EL MODELO
# ==========================================

predicciones = red.predict(X_test_norm)

exactitud = accuracy_score(
    y_test,
    predicciones
)

print("\n==============================")
print(" RESULTADOS RED NEURONAL")
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
# 7. CONVERTIR RED NEURONAL A C++
# ==========================================

# micromlgen no admite directamente
# MLPClassifier. Por eso exportamos
# los pesos y sesgos del modelo.

def numero(valor):

    if not np.isfinite(valor):
        return "0.0f"

    return f"{float(valor):.9g}f"


def vector(nombre, valores):

    elementos = ", ".join(
        numero(v) for v in valores
    )

    return (
        f"  const float {nombre}"
        f"[{len(valores)}] = "
        f"{{{elementos}}};\n"
    )


# ==========================================
# 8. GENERAR ARCHIVO red_neuronal.h
# ==========================================

with open(
    "red_neuronal.h",
    "w",
    encoding="utf-8"
) as archivo:

    archivo.write(
        "#pragma once\n"
        "#include <math.h>\n\n"
    )

    archivo.write(
        "namespace TinyIA {\n"
        "class RedNeuronal {\n"
        "public:\n"
    )

    archivo.write(
        "  int predict(const float entrada[4]) const {\n"
    )

    # Parametros de normalizacion
    archivo.write(
        vector("media", escalador.mean_)
    )

    archivo.write(
        vector("escala", escalador.scale_)
    )

    archivo.write(
        "  float x[4];\n"
        "  for (int i=0; i<4; ++i) "
        "x[i] = (entrada[i]-media[i])/escala[i];\n"
    )

    # Capa oculta de 6 neuronas
    for j in range(6):

        archivo.write(
            vector(
                f"w{j}",
                red.coefs_[0][:, j]
            )
        )

        pesos = numero(
            red.intercepts_[0][j]
        )

        archivo.write(
            f"  float h{j} = tanhf("
            f"x[0]*w{j}[0]+"
            f"x[1]*w{j}[1]+"
            f"x[2]*w{j}[2]+"
            f"x[3]*w{j}[3]+"
            f"{pesos});\n"
        )

    # Capa de salida
    salida = " + ".join(
        f"h{j}*{numero(red.coefs_[1][j, 0])}"
        for j in range(6)
    )

    archivo.write(
        f"  float z = {salida} + "
        f"{numero(red.intercepts_[1][0])};\n"
    )

    archivo.write(
        "  return z >= 0.0f ? 1 : 0;\n"
        "}\n"
        "};\n"
        "}\n"
    )


# ==========================================
# 9. RESULTADO FINAL
# ==========================================

print("\n==============================")
print("ARCHIVO GENERADO CORRECTAMENTE")
print("red_neuronal.h")
print("==============================")
