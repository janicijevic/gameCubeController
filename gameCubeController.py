import serial, re
import pyvjoy as vjoy

class Controller:
    def __init__(self, id):
        self.buttons = {"A": 0, "B": 0, "Start": 0, "X": 0, "Y": 0, "Z": 0, "L": 0, "R": 0, "Up": 0, "Down": 0, "Left": 0, "Right": 0}
        self.joy = {"X": 0x4000, "Y": 0x4000}       #Left joystick
        self.cstick = {"X": 0x4000, "Y": 0x4000}    #C-stick
        self.shoulder = {"Left": 0, "Right": 0}     #Shoulder buttons
        self.id = id
        self.j = vjoy.VJoyDevice(id)


    def setButtons(self, l):
        #Parse message
        self.buttons["A"] = int(l[7])
        self.buttons["B"] = int(l[6])
        self.buttons["X"] = int(l[5])
        self.buttons["Y"] = int(l[4])
        self.buttons["Start"] = int(l[3])
        self.buttons["L"] = int(l[9])
        self.buttons["R"] = int(l[10])
        self.buttons["Z"] = int(l[11])
        self.buttons["Up"] = int(l[12])
        self.buttons["Down"] = int(l[13])
        self.buttons["Right"] = int(l[14])
        self.buttons["Left"] = int(l[15])
        self.joy["X"] = scale(l[16:24], 2)
        self.joy["Y"] = scale(l[24:32], 2)
        self.cstick["X"] = scale(l[32:40], 1.5)
        self.cstick["Y"] = scale(l[40:48], 1.5)
        self.shoulder["Left"] = getHex(l[48:56])
        self.shoulder["Right"] = getHex(l[56:64])
        self.pushButtons()


    def pushButtons(self):
        #Send buttons pressed to appropriate controller
        self.j.data.wAxisX = self.joy["X"]
        self.j.data.wAxisY = self.joy["Y"]
        self.j.data.wAxisXRot = self.cstick["X"]
        self.j.data.wAxisYRot = self.cstick["Y"]
        self.j.data.wSlider = self.shoulder["Left"]
        self.j.data.wDial = self.shoulder["Right"]
        btns = 0
        for i, k in enumerate(self.buttons.keys()):
            btns |= self.buttons[k] << i
        self.j.data.lButtons = btns
        return self.j.update()



def scale(n, factor):
    #Scale value by given factor
    return int((getHex(n)-0x4000)*factor)+0x4000

def getHex(s):
    return int(s, 2)*0x80


def main():
    #Initialize controllers
    cs = [Controller(1), Controller(2)]
    #Start serial connection
    ser = serial.Serial("COM3", baudrate=115200)

    #Get initial message
    line = ser.readline()
    if(line[-7:] == b'ready\r\n'):
        print("Ready")
    
    
    while True:
        #Get message
        line = ser.readline()
        try:
            line = line.decode()
        except UnicodeDecodeError:
            print("Couldn't decode message. Retrying...")
            continue
        if not re.match(r"[ABCD]([10]){64}-", line): continue

        #See which controller is sending message
        index = ord(line[:1]) - ord("A")
        #Parse message and push buttons
        line = line[1:line.find('-')]
        cs[index].setButtons(line)
    


if __name__ == "__main__":
    main()

