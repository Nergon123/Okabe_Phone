import fuse
import os
import sys
import serial
from fuse import FUSE, Operations

port = "/dev/ttyACM1"  
BAUDRATE = 1000000

class SerialFS(Operations):
    def __init__(self, port="/dev/ttyUSB0", baudrate=9600):
        self.ser = serial.Serial(port, baudrate, timeout=1)

    def readdir(self, path, fh):
        return [".", "..", "serial_data"]

    def getattr(self, path, fh=None):
        return {"st_mode": 0o100644, "st_nlink": 1, "st_size": 1024}

    def read(self, path, size, offset, fh):
        if path == "/serial_data":
            return self.ser.read(size)
        return b""

    def write(self, path, data, offset, fh):
        if path == "/serial_data":
            self.ser.write(data)
            return len(data)
        return 0

def list_remote_files(path):
    
    global port
    with serial.Serial(port, BAUDRATE, timeout=1) as ser:
        print("WAITING FOR DEVICE...")  
        
        while True:
            response = ser.readline().decode("latin-1").strip()
            if response == "READY":
                break
        ser.write(("LIST" + path + "\n").encode())
        result = ser.read_until("**END").decode('utf-8') 
        try:
            result_ = result[result.index("**FILES:")+8:result.index("**END")]
        except:
            print("ERROR FROM ESP32:" + result)
        result_.strip()
        reslist = result_.split("\r\n")

        reslist = [item.strip() for item in reslist if item.strip()]
        return [".",".."].append(reslist)

def get_file(remote_path):
    print(f"Getting file: {remote_path}")
    global port
    try:
        with serial.Serial(port, BAUDRATE, timeout=1) as ser:
            print("WAITING FOR DEVICE...")  

            
            while True:
                response = ser.readline().decode("latin-1").strip()
                if response == "READY":
                    break

            
            ser.write(("SEND" + remote_path + "\n").encode())

            print(f"Requesting {remote_path} from device...")

            
            while True:
                response = ser.readline().decode("latin-1").strip()
                if response == "START":
                    break

            
            with open(remote_path.split("/")[-1], "wb") as file:  
                print(f"Receiving {remote_path}...")

                
                while True:
                    chunk = ser.read(512)
                    if not chunk:
                        break  
                    file.write(chunk)  
                    ser.write(b"ACK")  

            print(f"File {remote_path} received successfully.")

    except serial.SerialException as e:
        print(f"Error with the serial connection: {e}")
    except Exception as e:
        print(f"An error occurred: {e}")

mountpoint = sys.argv[1]
if not mountpoint:
    raise RuntimeError("No mountpoint")
FUSE(SerialFS(), mountpoint, foreground=True)