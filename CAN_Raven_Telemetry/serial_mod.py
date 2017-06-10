import serial
from tkinter import *

BRAKE_SIGNAL = 0;
HAZARD_SIGNAL = 1;
HEARTBEAT_SIGNAL = 6;
DIR_SIGNAL = 9;
BMS_CORE_STATUS = 10;

STATES = ["IDLE", "PRECHARGING", "RUNNING", "CHARGING", "SETUP"];

ERRORS = ["NONE", "SETTINGS CORRUPTED", "OVERCURRENT WARNING", "OVERCURRENT SHUTDOWN", \
          "LOW CELL VOLTAGE WARNING", "LOW CELL VOLTAGE SHUTDOWN", "HIGH CELL VOLTAGE WARNING", "HIGH CELL VOLTAGE SHUTDOWN", \
          "BMS OVERTEMPERATURE", "LOW STATE OF CHARGE WARNING", "TEMPERATURE EXCEEDED WARNING LEVEL", "CHASSIS LEAKAGE", \
          "AUX BATTERY VOLTAGE BELOW WARNING LEVEL", "PRECHARGE FAILED", "CONTATOR SWITCH ERROR", "CANBUS COMMUNICATION ERROR"];

connected = False;

while not connected:
        print("Enter COM port: ");
        port_num = input();

        try:
                ser = serial.Serial('COM' + port_num, 9600);
                connected = True;

        except:
                print("COM" + port_num +" is not connected\n");
        
root = Tk();



brake = StringVar();
brake.set("OFF");
hazard = StringVar();
hazard.set("OFF");

brake_frame = Frame(root);
brake_frame.grid(row = 0);

brake_label_caption =  Label(brake_frame, text = "Brake: " , font = (40), width = 25);
brake_label_caption.grid(row = 0, column = 0);
brake_label = Label(brake_frame, text = brake, textvariable = brake, font = 60, width = 25);
brake_label.grid(row = 0, column = 1);

hazard_label_caption =  Label(brake_frame, text = "Hazard: " , font = (40), width = 25);
hazard_label_caption.grid(row = 0, column = 2);
hazard_label = Label(brake_frame, text = hazard, textvariable = hazard, font = 40, width = 25);
hazard_label.grid(row = 0, column = 3);

dir_frame = Frame(root);
dir_frame.grid(row = 1, column = 0);

left_signal = StringVar();
left_signal.set("OFF");
right_signal = StringVar();
right_signal.set("OFF");

left_label_caption = Label(dir_frame, text = "Left Signal: " , font = (40), width = 25);
left_label_caption.grid(row = 0, column = 0);
left_label = Label(dir_frame, text = left_signal, textvariable = left_signal, font = (40), width = 25);
left_label.grid(row = 0, column = 1);

right_label_caption = Label(dir_frame, text = "Right Signal: ", font = (40), width = 25);
right_label_caption.grid(row = 0, column = 2);
right_label = Label(dir_frame, text = right_signal, textvariable = right_signal, font = (40), width = 25);
right_label.grid(row = 0, column = 3);

status = StringVar();
status.set("N/A");
error = StringVar();
error.set(ERRORS[0]);
state_of_charge = StringVar();
state_of_charge.set("N/A");
voltage = StringVar();
voltage.set("N/A");
current = StringVar();
current.set("N/A");
aux_voltage = StringVar();
aux_voltage.set("N/A");
temperature = StringVar();
temperature.set("N/A");

status_frame1 = Frame(root);
status_frame1.grid(row = 2, column = 0);
status_label_caption = Label(status_frame1, text = "Status: ",font = 40, width = 15);
status_label_caption.grid(row = 0, column = 0);
status_label = Label(status_frame1, text = status, textvariable = status, font = (40), width = 35);
status_label.grid(row = 0, column = 1);

error_label_caption = Label(status_frame1, text = "Error: ", font = 40, width = 15);
error_label_caption.grid(row = 0, column = 2);
error_label = Label(status_frame1, text = error, textvariable = error, font = (40), width = 35);
error_label.grid(row = 0, column = 3);

status_frame2 = Frame(root);
status_frame2.grid(row = 3, column = 0);
soc_label_caption = Label(status_frame2, text = "State of Charge: (%) ", font = 40, width = 20);
soc_label_caption.grid(row = 1, column = 0);
soc_label = Label(status_frame2, text = state_of_charge, textvariable = state_of_charge, font = (40), width = 20);
soc_label.grid(row = 2, column = 0);

voltage_label_caption = Label(status_frame2, text = "Voltage: (V)", font = 40, width = 20);
voltage_label_caption.grid(row = 1, column = 1);
voltage_label = Label(status_frame2, text = voltage, textvariable = voltage, font = (40), width = 20);
voltage_label.grid(row = 2, column = 1);

voltage_label_caption = Label(status_frame2, text = "Current: (A)", font = 40, width = 20);
voltage_label_caption.grid(row = 1, column = 2);
voltage_label = Label(status_frame2, text = current, textvariable = current, font = (40), width = 20);
voltage_label.grid(row = 2, column = 2);

voltage_label_caption = Label(status_frame2, text = "Aux Voltage: (V)", font = 40, width = 20);
voltage_label_caption.grid(row = 1, column = 3);
voltage_label = Label(status_frame2, text = aux_voltage, textvariable = aux_voltage, font = (40), width = 20);
voltage_label.grid(row = 2, column = 3);

voltage_label_caption = Label(status_frame2, text = "Temperature: (Cel.)", font = 40, width = 20);
voltage_label_caption.grid(row = 1, column = 4);
voltage_label = Label(status_frame2, text = temperature, textvariable = temperature, font = (40), width = 20);
voltage_label.grid(row = 2, column = 4);

battery_frame = Frame(root);
battery_frame.grid(row = 4, column = 0);
pack_details = [];


for i in range(4):
        pack_frame = Frame(battery_frame, width = 50);
        pack_frame.grid(row = int(i/2), column = i%2);
        title_frame = Frame(pack_frame);
        title_frame.grid(row = 0);
        detail_frame = Frame(pack_frame);
        detail_frame.grid(row = 1);
        Label(title_frame, text = "Pack " + str(i + 1) + ":", font = 40, width = 50).grid(row = 0);
        Label(detail_frame, text = "Cell: " , font = 40, width = 5).grid(row = 0, column = 0);
        Label(detail_frame, text = "Voltage: (V)" , font = 40, width = 15).grid(row = 0, column = 1);
        Label(detail_frame, text = "Volt warning: " , font = 40, width = 15).grid(row = 0, column = 2);
        Label(detail_frame, text = "Shun warning: " , font = 40, width = 15).grid(row = 0, column = 3);
        pack_details.append([]);

        for j in range(12):
                Label(detail_frame, text = str(j) + ":", font = 40, width = 12).grid(row = j + 1, column = 0);
                pack_details[i].append((StringVar(), StringVar(), StringVar(),));
                pack_details[i][j][0].set("N/A");
                pack_details[i][j][1].set("OK");
                pack_details[i][j][2].set("OK");
                Label(detail_frame, text = pack_details[i][j][0], textvariable = pack_details[i][j][0], font = 40, width = 14).grid(row = j + 1, column = 1);
                Label(detail_frame, text = pack_details[i][j][1], textvariable = pack_details[i][j][1] , font = 40, width = 12).grid(row = j + 1, column = 2);
                Label(detail_frame, text = pack_details[i][j][2], textvariable = pack_details[i][j][2] , font = 40, width = 12).grid(row = j + 1, column = 3);

def update(log_msg):
        timestamp_end = log_msg.find("]")+1;
        data_start = log_msg.find("}")+1;
        id = int(log_msg[log_msg.find("{")+1:log_msg.find("}")]);#are we sure this parsing will work? think it might cause problems for values between 0-9

        if id == BRAKE_SIGNAL:
                brake.set("ON" if int(log_msg[data_start]) else "OFF");
                print(log_msg[2:timestamp_end] + "[UPDATE] Brake: " + brake.get());

        elif id == HEARTBEAT_SIGNAL:


        elif id == HAZARD_SIGNAL:
                hazard.set("ON" if int(log_msg[data_start]) else "OFF");
                print(log_msg[2:timestamp_end] + "[UPDATE] Hazard: " + hazard.get());
        
        elif id == DIR_SIGNAL:
                left_signal.set("ON" if int(log_msg[data_start]) else "OFF");
                right_signal.set("ON" if int(log_msg[data_start + 1]) else "OFF");
                print(log_msg[2:timestamp_end] + "[UPDATE] Left Signal: " + left_signal.get() + "\tRight Signal: " + right_signal.get());

        elif id == BMS_CORE_STATUS:
                values = log_msg.split("] ")[1].split(" ");
                status.set(STATES[int(values[0])]);
                error.set(ERRORS[int(values[1])]);
                state_of_charge.set(int(values[2]));
                voltage.set(int(values[3]));
                current.set(int(values[4]));
                aux_voltage.set(int(values[5])/10.0);
                temperature.set(int(values[6][:len(values[6]) - 5]));
                print(log_msg[2:timestamp_end] + ("[WARNING]" if (int(values[1]) in [2,4,6,9]) else "[ERROR]") +\
                      " Status: " + STATES[int(values[0])] + " Error: " + ERRORS[int(values[1])] + \
                      " SoC: " + values[2] + "% Voltage: " + values[3] + "V Current: " + values[4] +\
                      "A Aux Voltage: " + str(int(values[5])/10.0) + "V Temperature: " + \
                      values[6][:len(values[6]) - 5]);
                
       # elif id >= 100 and id < 140:
                




def wait():
        log_msg = str(ser.readline());
        print(log_msg);

        if log_msg.find("] ") > -1:
                update(log_msg);
   
        root.after(100, wait);

root.after(10, wait);
root.mainloop();
