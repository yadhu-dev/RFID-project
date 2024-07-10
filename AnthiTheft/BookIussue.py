from flask import Flask, render_template, jsonify, request
import serial
import psycopg2

app = Flask(__name__)

port = 'COM3'
baud_rate = 9600
rollNo = ""
book_name = ""
uid = ""
uid_bookid = ""
ser = None

# Database Connections
db_name_books = 'bookid'  
db_name_students = 'rfid' 
user_name = 'postgres'
password = 'yADHU@2108'
local_host = 'localhost'
port_no = '5432'

def Initialize_Serial():
    global ser
    try:
        ser = serial.Serial(port, baud_rate)
        print(f"Successfully connected to {port} at {baud_rate} baud rate.")
    except serial.SerialException as e:
        print(f"Error on connection: {e}")

def read_serial():
    global ser, uid
    if ser and ser.in_waiting > 0:
        try:
            uid = ser.readline().decode('utf-8').strip()
            print(f"UID: {uid}")
        except Exception as e:
            print(f"Error in reading: {e}")
    return uid

def ConnectToDatabase(dbname):
    conn = psycopg2.connect(
        dbname=dbname,
        host=local_host,
        user=user_name,
        password=password,
        port=port_no,
    )
    return conn

@app.route('/')
def issue():
    global ser, uid, rollNo, book_name

    if not ser:
        Initialize_Serial()

    return render_template('Issue.html', rollNo=rollNo, book_name=book_name)

@app.route('/fetch_book_data', methods=['GET'])
def fetch_book_data():
    global uid, book_name, uid_bookid

    uid = read_serial()

    if not uid:
        return jsonify({'book_name': book_name})

    conn_books = ConnectToDatabase(db_name_books)
    cur_books = conn_books.cursor()
    cur_books.execute("SELECT book_name, book_id FROM books_uid WHERE book_id = %s", (uid,))
    uid_bookid_result = cur_books.fetchone()

    if uid_bookid_result:
        book_name = uid_bookid_result[0]
        uid_bookid = uid_bookid_result[1]
    else:
        book_name = "Not found"
        uid_bookid = ""

    cur_books.close()
    conn_books.close()

    return jsonify({'book_name': book_name})

@app.route('/fetch_student_data', methods=['GET'])
def fetch_student_data():
    global uid, rollNo

    uid = read_serial()

    if not uid:
        return jsonify({'rollNo': rollNo})

    conn_students = ConnectToDatabase(db_name_students)
    cur_students = conn_students.cursor()
    cur_students.execute("SELECT rollno FROM students WHERE uid = %s", (uid,))
    uid_rollNo = cur_students.fetchone()

    if uid_rollNo:
        rollNo = uid_rollNo[0]
    else:
        rollNo = "Not found"

    cur_students.close()
    conn_students.close()

    return jsonify({'rollNo': rollNo})

@app.route('/submit', methods=['POST'])
def submit():
    global uid_bookid, book_name

    book_name = request.form["book_name"]
    rollNo = request.form["roll_no"]

    conn_books = ConnectToDatabase(db_name_books)
    cur_books = conn_books.cursor()

    try:
        cur_books.execute("UPDATE books_uid SET issued_to = %s WHERE book_id = %s", (rollNo, uid_bookid))
        conn_books.commit()
        return jsonify({'success': True, 'message': 'Successfully Issued'})
    except Exception as e:
        conn_books.rollback()
        return jsonify({'success': False, 'message': str(e)})
    finally:
        cur_books.close()
        conn_books.close()

if __name__ == '__main__':
    app.run(debug=True)
