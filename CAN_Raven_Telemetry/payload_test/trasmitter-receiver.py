import serial
import time

connected = False;

while not connected:
        print("Enter transmitting COM port: ");
        port_num = input();

        try:
                ser = serial.Serial('COM' + port_num, 115200);
                connected = True;

        except:
                print("COM" + port_num +" is not connected\n");

connected = False;

def send_payload(payload):
    if len(payload.split(" ")) is not 2:
        print("Payload is not properly formatted");
        print("<id> <payload in hex>");
        return;

    try:
        id = int(payload.split(" ")[0]);
    except:
        print("invalid id");
        return;
    data = payload.split(" ")[1];

    if id >= 1000:
        print("invalid id");
        return;

    if len(data) < 16:
        for i in range(16 - len(data)):
            data += "0";
    ser.write(bytes([id, int(data[0:2], 16), int(data[2:4], 16), int(data[4:6], 16), int(data[6:8], 16),\
                     int(data[8:10], 16), int(data[10:12], 16), int(data[12:14], 16), int(data[14:16], 16)]));

while not connected:
    print("Testing GUI?");

    if (input() == 'y'):
        break;
    else:
        print("Enter receiving COM port: ");
        port_num = input();

        try:
           rec = serial.Serial('COM' + port_num, 9600);
           connected = True;
        except:
            print("COM" + port_num +" is not connected\n"); 

time.sleep(2);

while ser.inWaiting():
        print(ser.readline());
if connected:
    while rec.inWaiting():
        print(rec.readline());

print("\n\nRun auto test?");
if input() == 'y':
    for i in range(8):
        for j in range(8):
            for k in [0, 1, 2, 4, 5, 6, 8, 9, "a"]:
                send_payload("131 0" + str(i) + "00000" + str(j) + str(k));
                time.sleep(0.5);
                while ser.inWaiting():
                    print(ser.readline());
                if connected:
                    while rec.inWaiting():
                        print(rec.readline());
    for i in range(8):
        for j in range(8):
            for k in [0, 1, 2, 4, 5, 6, 8, 9, "a"]:
                send_payload("131 00" + str(i) + "0000" + str(j) + str(k));
                time.sleep(0.5);
                while ser.inWaiting():
                    print(ser.readline());
                if connected:
                    while rec.inWaiting():
                        print(rec.readline());
    print("-------------------TEST COMPLETED------------------------");
    

while True:
    while ser.inWaiting():
        print(ser.readline());
    if connected:
        while rec.inWaiting():
            print(rec.readline());

    payload = input();
    send_payload(payload);
    time.sleep(0.5);


