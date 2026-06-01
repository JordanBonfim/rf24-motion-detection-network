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

def ler_serial_arduino(ser):
    print(" Waiting movement...")
    while True:
        try:
            if ser and ser.in_waiting > 0:
                data = ser.readline().decode('utf-8').strip()
                if data:
                    timestamp = datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")
                    print(f"Movement: {data} ({timestamp})")

                    socketio.emit('motion_detected', {
                        'message': data,
                        'timestamp': timestamp
                    })
            time.sleep(0.05) 
        except Exception as e:
            print(f"Error reading serial: {e}")
            break

@app.route('/')
def index():
    return render_template('index.html')

# route for receiving commands from the web 
@socketio.on('send_command')
def handle_comando(dados):
    command = dados.get('command')
    if ser and ser.is_open:
        ser.write(command.encode('utf-8'))
        print(f"Command sent to Serial: {command}")

if __name__ == '__main__':
    
    ser = None
    # Inicializa a conexão serial
    try:
        ser = serial.Serial(PORTA_SERIAL, BAUD_RATE, timeout=1)
        print(f"Connected to Arduino on port {PORTA_SERIAL}!")
        time.sleep(2)
    except serial.SerialException as e:
        print(f" Error opening serial port: {e}")
        sys.exit(1)
    
    thread_serial = threading.Thread(target=ler_serial_arduino, args=(ser,), daemon=True)
    thread_serial.start()
    
    # web server
    print("Starting web server at http://localhost:5000")
    socketio.run(app, debug=True, use_reloader=False)