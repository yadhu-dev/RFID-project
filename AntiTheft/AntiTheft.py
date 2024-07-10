import serial
import psycopg2


port = 'COM3'
baud_rate = 9600


db_name1 = 'bookid'
user_name1 = 'postgres'
password1 = 'yADHU@2108'
local_host1 = 'localhost'
port_no1 = '5432'


def send_to_arduino(ser, data):
    ser.write(data.encode() + b'\n')


def connect_to_db():
    return psycopg2.connect(
        dbname=db_name1,
        user=user_name1,
        password=password1,
        host=local_host1,
        port=port_no1
    )

def main():
    ser = None
    conn = None

    try:
        
        ser = serial.Serial(port, baud_rate)
        print(f"Your program is connected to {port} at {baud_rate} baud rate...")

        while True:
           
            if ser.in_waiting > 0:
                uid = ser.readline().decode('utf-8').rstrip()
                print(f"UID received: {uid}")

                
                conn = connect_to_db()
                cur = conn.cursor()

              
                cur.execute("SELECT * FROM books_uid WHERE book_id = %s", (uid,))
                existing_uid = cur.fetchone()

                if existing_uid:
                    send_to_arduino(ser, "GREEN")
                    print("Data found: Green LED should blink...")
                else:
                    send_to_arduino(ser, "RED")
                    print("Data not found: Red LED should blink...")

                
                cur.close()
                conn.close()

    except Exception as e:
        print(f"Error: {e}")

    finally:
        
        if conn:
            conn.close()
            print("Database connection closed.")
       
        if ser:
            ser.close()
            print("Serial connection closed.")


if __name__ == '__main__':
    main()
