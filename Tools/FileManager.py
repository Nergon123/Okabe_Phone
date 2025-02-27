import serial
import time
import os
import platform
from tqdm import tqdm  
from simple_term_menu import TerminalMenu

port = "/dev/ttyACM1"  #default port
BAUDRATE = 1000000 

def main_menu():
    clear_terminal()
    global port
    print("MAIN MENU")
    options = ["SEND FILES", "REMOTE FILE MANAGER", "EXIT"]
    while True:
        menuIndex = TerminalMenu(options).show()
        if menuIndex == 0:
            choose_local_file()
        elif menuIndex == 1:
            remote_files()
        elif menuIndex == 2:
            with serial.Serial(port, BAUDRATE, timeout=1) as ser:
                ser.write(("EXIT").encode())
            return

def remote_files():
    
    path = "/"
    exit = False
    while not exit:
        clear_terminal()
        if path == "":
            path = "/"
        print("Current Path: " + path)  
        try:
            reslist = ["../"] + list_remote_files(path)
            menuIndex = TerminalMenu(reslist).show()  

            if reslist[menuIndex] == "../":  
                if path != "/":
                    path = path[:path.rindex("/")]  
                else:
                    exit = True
                
            elif reslist[menuIndex].endswith("/"):  
                path += '/'
                path += reslist[menuIndex].replace("/", "")  
                path = path.replace("//", "/")  

            else:  
                file_menu = TerminalMenu(["CANCEL", "GET FILE", "DELETE FILE"]).show()

                if file_menu == 1:
                    get_file(path + reslist[menuIndex])  
                elif file_menu == 2:
                    delete_file(path + reslist[menuIndex])

        except Exception as e:
            print(e)
            path = "/"
            time.sleep(2)

    main_menu()


def get_file(remote_path):
    clear_terminal()
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
                    ser.write(b"ACK\n")  

            print(f"File {remote_path} received successfully.")

    except serial.SerialException as e:
        print(f"Error with the serial connection: {e}")
    except Exception as e:
        print(f"An error occurred: {e}")

def clear_terminal():
    if platform.system().lower() == 'windows':
        os.system('cls')
    else:
        os.system('clear')


def delete_file(remote_path):
    clear_terminal()
    print(f"Deleting file: {remote_path}")
    
    with serial.Serial(port, BAUDRATE, timeout=1) as ser:
        while True:
            response = ser.readline().decode("latin-1").strip()
            if response == "READY":
                break

        
        ser.write(("DELETE" + remote_path + "\n").encode())
        

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
        return reslist



def choose_local_file():
    exit = False
    while not exit:
        clear_terminal()
        dir = []
        dir.append("..")
        filelist = [".."] + os.listdir()  
        for f in os.listdir():
            if os.path.isdir(f):
                dir.append(f + "/")
            else:
                dir.append(f)

        options = dir
        terminal_menu = TerminalMenu(options)
        menu_entry_index = terminal_menu.show()
        file = filelist[menu_entry_index]

        if os.path.isdir(file):
            os.chdir(file)
        else:
            me_index = TerminalMenu(["SEND TO DEVICE", "CANCEL"]).show()
            if me_index == 0:
                if port and file:
                    send_file(port, file)
                    exit = True
                else:
                    print("Port or file not set.")

def ta_request(request: str, variable: str):
    variable_ = variable
    variable = input(request)
    return variable if variable else variable_

def send_file(serial_port, file_path):
    clear_terminal()
    file_name = os.path.basename(file_path)
    file_size = os.path.getsize(file_path)

    with serial.Serial(serial_port, BAUDRATE, timeout=1) as ser:
        print("WAITING FOR DEVICE...")  
        
        while True:
            response = ser.readline().decode("latin-1").strip()
            if response == "READY":
                break
        
        ser.write(("RECIEVE" + file_name + "\n").encode())
        
        while True:
            response = ser.readline().decode("latin-1").strip()
            if response == "START":
                break

        with open(file_path, "rb") as f, tqdm(
            total=file_size, unit="B", unit_scale=True, desc="Uploading"
        ) as progress:
            while chunk := f.read(512):  
                ser.write(chunk)
                ser.flush()  
                time.sleep(0.001)  

                while True:
                    ack = ser.readline().decode("latin-1").strip()
                    if ack == "ACK":
                        break

                progress.update(len(chunk))
        
        while True:
            response = ser.readline().decode("latin-1").strip()
            if response == "DONE":
                print("File transfer complete!")
                break


port = ta_request("COM PORT [" + port + "]?", port)
main_menu()
