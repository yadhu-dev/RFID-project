from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import serial
import threading
import time
import atexit
import os

app = Flask(__name__)
socketio = SocketIO(app)

# Variable to store the serial port instance and the latest RFID data
arduino = None
rfid_data = ""

def init_serial():
    global arduino
    try:
        if arduino is None or not arduino.is_open:
            arduino = serial.Serial('COM6', 9600, timeout=1)  # Replace with your COM port
            time.sleep(2)  # Allow time for the connection to establish
            print("Serial port initialized successfully")
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        arduino = None

def read_from_arduino():
    global rfid_data
    while True:
        if arduino and arduino.is_open:
            arduino.write(b'Read\n')  # Send read command
            time.sleep(0.1)  # Short delay before reading data

            if arduino.in_waiting > 0:
                line = arduino.readline().decode('utf-8').strip()
                if line:
                    print(f"Received data: {line}")
                    rfid_data = line
                    # Emit data to WebSocket clients
                    socketio.emit('rfid_update', {'data': rfid_data})
                    
        time.sleep(1)  # Adjust sleep time as needed

def close_serial():
    global arduino
    if arduino and arduino.is_open:
        arduino.close()
        print("Serial port closed")

atexit.register(close_serial)

# Only initialize the serial port if this is not a reloader process
if os.environ.get('WERKZEUG_RUN_MAIN') == 'true':
    try:
        init_serial()
        # Start the background thread for reading from the Arduino
        thread = threading.Thread(target=read_from_arduino)
        thread.daemon = True
        thread.start()
    except Exception as e:
        print(f"Error during initialization: {e}")

@app.route('/')
def index():
    return render_template('index.html')

@socketio.on('connect')
def handle_connect():
    emit('rfid_update', {'data': rfid_data})

if __name__ == '__main__':
    socketio.run(app, debug=True)
