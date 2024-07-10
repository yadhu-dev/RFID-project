from flask import Flask, render_template, request, redirect, url_for, jsonify
import serial
import psycopg2

app = Flask(__name__)

# Serial port configuration
serial_port = 'COM3'
baud_rate = 9600

ser = None
book_id = ""
EMAIL = 'kjc@kristujayanti.com'
PASSWORD_LOGIN = 'Kjc@123'

# Database configuration
db_name = 'bookid'
user_name = 'postgres'
password = 'yADHU@2108'
local_host = 'localhost'
db_port = '5432'
#------------------------------------


def initialize_connection():
    global ser
    try:
        ser = serial.Serial(serial_port, baud_rate)
        print(f"Connection established to {serial_port} at {baud_rate} baud rate....")
    except serial.SerialException as e:
        print(f"Connection Error: {e}")


def read_serial():
    global ser, book_id
    try:
        if ser and ser.in_waiting > 0:
            book_id = ser.readline().decode('utf-8').strip()
            print(book_id)
    except Exception as e:
        print(f"Reading Error: {e}")
    return book_id


def connect_to_database():
    try:
        conn = psycopg2.connect(
            dbname=db_name,
            host=local_host,
            user=user_name,
            password=password,
            port=db_port
        )
        return conn
    except psycopg2.OperationalError as e:
        print(f"Database Connection Error: {e}")
        return None


@app.route('/')
def login():
    return render_template('login.html')


@app.route('/login', methods=['POST'])
def dologin():
    email_id = request.form['email']
    password = request.form['password']
    if email_id == EMAIL and password == PASSWORD_LOGIN:
        return redirect(url_for('create'))
    else:
        error = 'Invalid Credentials. Please try again.'
        return render_template('login.html', error=error)


@app.route('/submit', methods=['POST'])
def assign():
    book_id = request.form['bookuid_ip']
    book_name = request.form['name_ip']
    if book_id and book_name:
        conn = connect_to_database()
        if conn:
            cur = conn.cursor()
            try:
                cur.execute("INSERT INTO books_uid (book_name, book_id) VALUES (%s, %s)", (book_name, book_id))
                conn.commit()
                return jsonify({'success': True, 'message': 'Submission successful'})
            except Exception as e:
                conn.rollback()
                return jsonify({'success': False, 'message': str(e)})
            finally:
                cur.close()
                conn.close()
        else:
            return jsonify({'success': False, 'message': 'Database connection failed'})


@app.route('/bookAssign')
def create():
    global book_id, ser
    if not ser:
        initialize_connection()
    book_id = read_serial()
    return render_template('Assign.html', book_id=book_id)


@app.route('/uid')
def update_uid():
    global book_id
    book_id = read_serial()
    return jsonify({'book_id': book_id})


if __name__ == '__main__':
    app.run(debug=True)
