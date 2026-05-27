from flask import Flask, render_template
from flask_socketio import SocketIO
import serial
import threading
import time
import datetime
import sys

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

PORTA_SERIAL = 'COM5' 
BAUD_RATE = 115200

def ler_serial_arduino():
    try:
        ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
        print(f"Conectado ao Arduino na porta {PORTA_SERIAL}!")
        time.sleep(2)
    except serial.SerialException as e:
        print(f" Erro ao abrir a porta serial: {e}")
        sys.exit(1)
    print(" Aguardadando movimento...")
    
    while True:
        try:
            if ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                
                if data:
                    timestamp = datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")
                    print(f"Movimento: {data} ({timestamp})")

                    socketio.emit('motion_detected', {
                        'mensagem': data,
                        'timestamp': timestamp
                    })
            time.sleep(0.05) 
        except Exception as e:
            print(f"Erro na leitura: {e}")
            break

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '__main__':
    # thread para ler arduino
    thread_serial = threading.Thread(target=ler_serial_arduino, daemon=True)
    thread_serial.start()
    
    # servidor web
    print("Iniciando servidor web em http://localhost:5000")
    socketio.run(app, debug=True, use_reloader=False)