import serial
import time
import struct

class comm:
    def __init__(self, port):
        try:
            self.arduino = serial.Serial(port, 9600)
        except serial.SerialException:
            print("couldn't open port")
        time.sleep(2)

    def passtoSerial(self, xcord, ycord):
        data = struct.pack('<hh', xcord, ycord)
        self.arduino.write(data)

    def checkresponse(self):
        response = self.arduino.readline().decode().strip()
        if len(response)>0:
            return response
        else: 
            return None