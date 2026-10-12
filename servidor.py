
from http.server import BaseHTTPRequestHandler, HTTPServer
from datetime import datetime
from html import escape
import json

# =====================================
# CONFIGURACION
# =====================================

PUERTO = 5000

notificaciones = []
ultimo_estado = None


# =====================================
# SERVIDOR TINY IA
# =====================================

class ServidorTinyIA(BaseHTTPRequestHandler):

    # =================================
    # PAGINA WEB
    # =================================

    def do_GET(self):

        if self.path != "/":
            self.send_error(404)
            return

        if ultimo_estado is None:

            estado = "SIN DATOS"
            temperatura = "-"
            distancia = "-"
            ventilador = "SIN DATOS"
            fecha = "-"

        else:

            estado = ultimo_estado["estado"]
            temperatura = ultimo_estado["temperatura"]
            distancia = ultimo_estado["distancia"]
            ventilador = ultimo_estado["ventilador"]
            fecha = ultimo_estado["fecha"]

        filas = ""

        for evento in reversed(notificaciones[-50:]):

            filas += f"""
            <tr>
                <td>{escape(str(evento['fecha']))}</td>
                <td>{escape(str(evento['estado']))}</td>
                <td>{escape(str(evento['temperatura']))}</td>
                <td>{escape(str(evento['distancia']))}</td>
                <td>{escape(str(evento['ventilador']))}</td>
            </tr>
            """

        color_ventilador = (
            "#16803c" if ventilador == "ON"
            else "#64748b"
        )

        pagina = f"""
        <!DOCTYPE html>
        <html lang="es">

        <head>
            <meta charset="UTF-8">
            <meta http-equiv="refresh" content="3">

            <title>Laboratorio Tiny IA</title>

            <style>

                body {{
                    font-family: Arial, sans-serif;
                    background: #f3f5f8;
                    color: #243447;
                    text-align: center;
                    padding: 25px;
                }}

                h1 {{
                    color: #263e57;
                }}

                .panel {{
                    display: flex;
                    flex-wrap: wrap;
                    justify-content: center;
                    gap: 15px;
                    margin: 25px auto;
                }}

                .tarjeta {{
                    background: white;
                    padding: 20px;
                    border-radius: 10px;
                    min-width: 170px;
                    box-shadow: 0 2px 8px #ddd;
                }}

                .valor {{
                    font-size: 24px;
                    font-weight: bold;
                    margin-top: 12px;
                }}

                table {{
                    margin: 20px auto;
                    width: 95%;
                    border-collapse: collapse;
                    background: white;
                }}

                th, td {{
                    border: 1px solid #ddd;
                    padding: 12px;
                }}

                th {{
                    background: #263e57;
                    color: white;
                }}

            </style>
        </head>

        <body>

            <h1>Laboratorio Tiny IA</h1>

            <h2>Deteccion y fusion sensorial</h2>

            <div class="panel">

                <div class="tarjeta">

                    <h3>Prediccion IA</h3>

                    <div class="valor">
                        {escape(str(estado))}
                    </div>

                </div>

                <div class="tarjeta">

                    <h3>Temperatura</h3>

                    <div class="valor">
                        {escape(str(temperatura))} °C
                    </div>

                </div>

                <div class="tarjeta">

                    <h3>Distancia</h3>

                    <div class="valor">
                        {escape(str(distancia))} cm
                    </div>

                </div>

                <div class="tarjeta">

                    <h3>Ventilador</h3>

                    <div class="valor"
                         style="color: {color_ventilador}">

                        {escape(str(ventilador))}

                    </div>

                </div>

            </div>

            <p>
                Ultima actualizacion:
                {escape(str(fecha))}
            </p>

            <p>
                Notificaciones recibidas:
                {len(notificaciones)}
            </p>

            <h2>Historial de detecciones</h2>

            <table>

                <tr>
                    <th>Fecha</th>
                    <th>Estado</th>
                    <th>Temperatura</th>
                    <th>Distancia</th>
                    <th>Ventilador</th>
                </tr>

                {filas}

            </table>

        </body>
        </html>
        """

        contenido = pagina.encode("utf-8")

        self.send_response(200)

        self.send_header(
            "Content-Type",
            "text/html; charset=utf-8"
        )

        self.end_headers()

        self.wfile.write(contenido)

    # =================================
    # RECIBIR NOTIFICACIONES
    # =================================

    def do_POST(self):

        global ultimo_estado

        if self.path != "/presencia":
            self.send_error(404)
            return

        try:

            longitud = int(
                self.headers.get("Content-Length", 0)
            )

            if longitud <= 0 or longitud > 4096:
                self.send_error(400)
                return

            contenido = self.rfile.read(longitud)

            datos = json.loads(
                contenido.decode("utf-8")
            )

            if not isinstance(datos, dict):
                self.send_error(400)
                return

            evento = {

                "fecha": datetime.now().strftime(
                    "%d/%m/%Y %H:%M:%S"
                ),

                "estado": datos.get(
                    "estado", "PRESENTE"
                ),

                "temperatura": datos.get(
                    "temperatura", "-"
                ),

                "distancia": datos.get(
                    "distancia", "-"
                ),

                "ventilador": datos.get(
                    "ventilador", "SIN DATOS"
                )

            }

            ultimo_estado = evento

            notificaciones.append(evento)

            print()
            print("NOTIFICACION RECIBIDA:")
            print(evento)

            respuesta = json.dumps({

                "mensaje": "Notificacion recibida"

            }).encode("utf-8")

            self.send_response(200)

            self.send_header(
                "Content-Type",
                "application/json"
            )

            self.end_headers()

            self.wfile.write(respuesta)

        except (ValueError, UnicodeDecodeError):

            self.send_error(
                400, "Datos invalidos"
            )


# =====================================
# INICIAR SERVIDOR
# =====================================

servidor = HTTPServer(
    ("0.0.0.0", PUERTO),
    ServidorTinyIA
)

print("================================")
print("SERVIDOR TINY IA INICIADO")
print("http://localhost:5000")
print("================================")

servidor.serve_forever()
