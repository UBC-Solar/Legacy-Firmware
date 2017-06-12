import serial
from tkinter import *

BRAKE_SIGNAL = 0;
HAZARD_SIGNAL = 1;
HEARTBEAT_SIGNAL = 6;
DIR_SIGNAL = 9;
BMS_CORE_STATUS = 10;
CURRENT_SIGNAL_1 = 202;
CURRENT_SIGNAL_2 = 201;
TEMP_SIGNAL_1 = 199;
TEMP_SIGNAL_2 = 198;
TEMP_SIGNAL_3 = 197;

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

brake_label_caption =  Label(brake_frame, text = "Brake: " , font = (None, 10,), width = 25);
brake_label_caption.grid(row = 0, column = 0);
brake_label = Label(brake_frame, text = brake, textvariable = brake, font = (None, 10,), width = 25);
brake_label.grid(row = 0, column = 1);

hazard_label_caption =  Label(brake_frame, text = "Hazard: " , font = (None, 10,), width = 25);
hazard_label_caption.grid(row = 0, column = 2);
hazard_label = Label(brake_frame, text = hazard, textvariable = hazard, font = (None, 10,),  width = 25);
hazard_label.grid(row = 0, column = 3);

dir_frame = Frame(root);
dir_frame.grid(row = 1, column = 0);

left_signal = StringVar();
left_signal.set("OFF");
right_signal = StringVar();
right_signal.set("OFF");

left_label_caption = Label(dir_frame, text = "Left Signal: " , font = (None, 10,), width = 25);
left_label_caption.grid(row = 0, column = 0);
left_label = Label(dir_frame, text = left_signal, textvariable = left_signal, font = (None, 10,), width = 25);
left_label.grid(row = 0, column = 1);

right_label_caption = Label(dir_frame, text = "Right Signal: ", font = (None, 10,), width = 25);
right_label_caption.grid(row = 0, column = 2);
right_label = Label(dir_frame, text = right_signal, textvariable = right_signal, font = (None, 10,) , width = 25);
right_label.grid(row = 0, column = 3);

acceleration = StringVar();
acceleration.set("N/A");

regen = StringVar();
regen.set("N/A");

direction = StringVar();
direction.set("FORWARD");

motor_frame = Frame(root);
motor_frame.grid(row = 2, column = 0);

acceleration_label_caption = Label(motor_frame, text = "Acceleration: ", font = (None, 10,), width = 25);
acceleration_label_caption.grid(row = 0, column = 0);
acceleration_label = Label(motor_frame, text = acceleration, textvariable = acceleration, font = (None, 10,), width = 25);
acceleration_label.grid(row = 0, column = 1);

regen_label_caption = Label(motor_frame, text = "Regen: ", font = (None, 10,), width = 25);
regen_label_caption.grid(row = 0, column = 2);
regen_label = Label(motor_frame, text = regen, textvariable = regen, font = (None, 10,), width = 25);
regen_label.grid(row = 0, column = 3);

direction_label_caption = Label(motor_frame, text = "Direction: ", font = (None, 10,), width = 25);
direction_label_caption.grid(row = 0, column = 4);
direction_label = Label(motor_frame, text = direction, textvariable = direction, font = (None, 10,), width = 25);
direction_label.grid(row = 0, column = 5);

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
status_frame1.grid(row = 3, column = 0);
status_label_caption = Label(status_frame1, text = "Status: ",font = (None, 10,), width = 15);
status_label_caption.grid(row = 0, column = 0);
status_label = Label(status_frame1, text = status, textvariable = status, font = (None, 10,), width = 35);
status_label.grid(row = 0, column = 1);

error_label_caption = Label(status_frame1, text = "Error: ", font = (None, 10,), width = 15);
error_label_caption.grid(row = 0, column = 2);
error_label = Label(status_frame1, text = error, textvariable = error, font = (None, 10,), width = 35);
error_label.grid(row = 0, column = 3);

car_details_frame = Frame(root);
car_details_frame.grid(row = 4);
#car_info_frame = Frame(details_frame);
#car_info_frame.grid(column = 0);

status_frame2 = Frame(car_details_frame);
status_frame2.grid(row = 0, column = 0);
soc_label_caption = Label(status_frame2, text = "State of Charge: (%) ", font = (None, 10,), width = 20);
soc_label_caption.grid(row = 0, column = 0);
soc_label = Label(status_frame2, text = state_of_charge, textvariable = state_of_charge, font = (None, 15,), width = 20, height = 3);
soc_label.grid(row = 1, column = 0);

voltage_label_caption = Label(status_frame2, text = "Voltage: (V)", font = (None, 10,), width = 20);
voltage_label_caption.grid(row = 0, column = 1);
voltage_label = Label(status_frame2, text = voltage, textvariable = voltage, font = (None, 15,), width = 20, height = 3);
voltage_label.grid(row = 1, column = 1);

voltage_label_caption = Label(status_frame2, text = "Current: (A)", font = (None, 10,), width = 20);
voltage_label_caption.grid(row = 2, column = 0);
voltage_label = Label(status_frame2, text = current, textvariable = current, font = (None, 15,), width = 20, height = 3);
voltage_label.grid(row = 3, column = 0);

voltage_label_caption = Label(status_frame2, text = "Aux Voltage: (V)", font = (None, 10,), width = 20);
voltage_label_caption.grid(row = 2, column = 1);
voltage_label = Label(status_frame2, text = aux_voltage, textvariable = aux_voltage, font = (None, 15,), width = 20, height = 3);
voltage_label.grid(row = 3, column = 1);

voltage_label_caption = Label(status_frame2, text = "Temperature: (Cel.)", font = (None, 10,), width = 20);
voltage_label_caption.grid(row = 4, column = 0);
voltage_label = Label(status_frame2, text = temperature, textvariable = temperature, font = (None, 15,), width = 20, height = 3);
voltage_label.grid(row = 5, column = 0);




#battery_frame = Frame(details_frame);
#battery_frame.grid(row = 0, column = 1);
pack_details = [];

for i in range(4):
        pack_frame = Frame(car_details_frame, width = 50);
        pack_frame.grid(row = int(i/2), column = i%2 + 1);
        title_frame = Frame(pack_frame);
        title_frame.grid(row = 0);
        detail_frame = Frame(pack_frame);
        detail_frame.grid(row = 1);
        Label(title_frame, text = "Pack " + str(i + 1) + ":", font = (None, 10,), width = 50).grid(row = 0);
        Label(detail_frame, text = "Cell: " , font = (None, 10,), width = 5).grid(row = 0, column = 0);
        Label(detail_frame, text = "Voltage: (V)" , font = (None, 10,), width = 10).grid(row = 0, column = 1);
        Label(detail_frame, text = "Volt warning: " , font = (None, 10,), width = 10).grid(row = 0, column = 2);
        Label(detail_frame, text = "Shun warning: " , font = (None, 10,), width = 10).grid(row = 0, column = 3);
        pack_details.append([]);

        for j in range(12):
                Label(detail_frame, text = str(j) + ":", font = (None, 10,), width = 12).grid(row = j + 1, column = 0);
                pack_details[i].append((StringVar(), StringVar(), StringVar(),));
                pack_details[i][j][0].set("N/A");
                pack_details[i][j][1].set("OK");
                pack_details[i][j][2].set("OK");
                Label(detail_frame, text = pack_details[i][j][0], textvariable = pack_details[i][j][0], font = (None, 10,), width = 14).grid(row = j + 1, column = 1);
                Label(detail_frame, text = pack_details[i][j][1], textvariable = pack_details[i][j][1] , font = (None, 10,), width = 12).grid(row = j + 1, column = 2);
                Label(detail_frame, text = pack_details[i][j][2], textvariable = pack_details[i][j][2] , font = (None, 10,), width = 12).grid(row = j + 1, column = 3);

        pack_temp_frame = Frame(pack_frame);
        pack_temp_frame.grid(row = 2);
        Label(pack_temp_frame, text = "Temperature:  ", font = (None, 10,), width = 20).grid(row = 0, column = 1);
        Label(pack_temp_frame, text = "Warning :", font = (None, 10,), width = 20).grid(row = 0, column = 2);

        for k in range(2):
                Label(pack_temp_frame, text = "Temperature " + str(k) + ": ", font = (None, 10,), width = 12).grid(row = k + 1);
                pack_details[i].append((StringVar(), StringVar(),));
                pack_details[i][k + 12][0].set("N/A");
                pack_details[i][k + 12][1].set("OK");
                Label(pack_temp_frame, text = pack_details[i][12 + k][0], textvariable = pack_details[i][12 + k][0], font = (None, 10,), width = 14).grid(row = k + 1, column = 1);
                Label(pack_temp_frame, text = pack_details[i][12 + k][1], textvariable = pack_details[i][12 + k][1], font = (None, 10,), width = 14).grid(row = k + 1, column = 2);

mppt_frame = Frame(car_details_frame, width = 50);
mppt_frame.grid(row= 1, column = 0, sticky = N);
mppt_title_frame = Frame(mppt_frame);

mppt_details_frame = Frame(mppt_frame);
mppt_details_frame.grid(row = 1);

mppt_title_frame.grid(row = 0);
Label(mppt_title_frame, text = "MPPT info", font = (None, 10,), width = 50).grid(row = 0);

mppt_current_frame = Frame(mppt_details_frame);
mppt_current_frame.grid(row = 1, column = 0, sticky = N);
mppt_current_title_frame = Frame(mppt_current_frame, width = 20);
mppt_current_title_frame.grid(row = 0);
Label(mppt_current_title_frame, text = "Current (mA)", font = (None, 10,), width = 15).grid(row = 0);
current_details = [];

for i in range(6):
        mppt_current_detail_frame = Frame(mppt_current_frame, width = 20);
        mppt_current_detail_frame.grid(row = i+1);
        Label(mppt_current_detail_frame, text = str(i) + ":", font = (None, 10,), width = 10).grid(row = 0, column = 0);
        current_details.append(StringVar());
        current_details[i].set("N/A");
        Label(mppt_current_detail_frame, text = current_details[i], textvariable = current_details[i], font = (None, 10,), width = 10).grid(row = 0, column = 1);

mppt_temp_frame = Frame(mppt_details_frame);
mppt_temp_frame.grid(row = 1, column = 1);
mppt_temp_title_frame = Frame(mppt_temp_frame, width = 20);
mppt_temp_title_frame.grid(row = 0);
Label(mppt_temp_title_frame, text = "Temp (C)", font = (None, 10,), width = 15).grid(row = 0);
temp_details = [];

for i in range(10):
        mppt_temp_detail_frame = Frame(mppt_temp_frame, width = 20);
        mppt_temp_detail_frame.grid(row = i+1);
        Label(mppt_temp_detail_frame, text = str(i) + ":", font = (None, 10,), width = 10).grid(row = 0, column = 0);
        temp_details.append(StringVar());
        temp_details[i].set("N/A");
        Label(mppt_temp_detail_frame, text = temp_details[i], textvariable = temp_details[i], font = (None, 10,), width = 10).grid(row = 0, column = 1);

                
def update(log_msg):
        timestamp_end = log_msg.find("]")+1;
        data_start = log_msg.find("}")+2;
        id = int(log_msg[log_msg.find("{")+1:log_msg.find("}")]);

        if id == BRAKE_SIGNAL:
                brake.set("ON" if int(log_msg[data_start]) else "OFF");
                print(log_msg[2:timestamp_end] + "[UPDATE] Brake: " + brake.get());

        elif id == HEARTBEAT_SIGNAL:
                acceleration.set(str(double(log_msg[log_msg.find("A")+1:log_msg("R")]) / 255.0 * 100.0));
                regen.set(str(double(log_msg[log_msg.find("R")+1:log_msg("D")]) / 255.0 * 100.0));
                direction.set("FORWARD" if int(log_msg[log_msg.find("D")+1:log_msg.find("S")]) else "REVERSE")

                signals = log_msg[log_msg.find("S")+1:log_msg.find("E")];
                hazard.set("ON" if int(signals[3]) else "OFF");
                left_signal.set("ON" if int(signals[7]) else "OFF");
                right_signal.set("ON" if int(signals[6]) else "OFF");
                print(log_msg[2:timestamp_end] + "[HEARTBEAT] ");
                print("Acceleration%: " + acceleration.get() + "\tRegen%: " + regen.get() + "\tDirection: " + direction.get());
                print("Left signal: " + left_signal.get() + "\tRight signal: " + right_signal.get() + "\tHazard: " + hazard.get());

        elif id == BMS_CORE_STATUS:
                values = log_msg.split("} ")[1].split(" ");
                status.set(STATES[int(values[0])%4]); #mod 4 is there just for testing. remove after
                error.set(ERRORS[int(values[1])]);
                state_of_charge.set(int(values[2]));
                voltage.set(int(values[3]));
                current.set(int(values[4]));
                aux_voltage.set(int(values[5])/10.0);
                temperature.set(int(values[6][:len(values[6]) - 5]));
                print(log_msg[2:timestamp_end] + ("[WARNING]" if (int(values[1]) in [2,4,6,9]) else "[ERROR]") +\
                      " Status: " + status.get() + " Error: " + ERRORS[int(values[1])] + \
                      " SoC: " + values[2] + "% Voltage: " + values[3] + "V Current: " + values[4] +\
                        "A Aux Voltage: " + aux_voltage.get() + "V Temperature: " + \
                        temperature.get());

        elif id == CURRENT_SIGNAL_1:
                currents = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT CURRENT1] ");
                for i in range(4):
                        current_details[i].set(currents[i+1]);
                        print("Current sensor #" + str(i) + ": " + current_details[i].get() + "mA");

        elif id == CURRENT_SIGNAL_2:
                currents = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT CURRENT2] ");
                for i in range(2):
                        current_details[i+4].set(currents[i+1]);
                        print("Current sensor #" + str(i+4) + ": " + current_details[i+4].get() + "mA");

        elif id == TEMP_SIGNAL_1:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP1] ");
                for i in range(4):
                        temp_details[i].set(temps[i+1]);
                        print("Temp sensor #" + str(i) + ": " + temp_details[i].get() + "C");

        elif id == TEMP_SIGNAL_2:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP2] ");
                for i in range(4):
                        temp_details[i+4].set(temps[i+1]);
                        print("Temp sensor #" + str(i+4) + ": " + temp_details[i+4].get() + "C");

        elif id == TEMP_SIGNAL_3:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP3] ");
                for i in range(2):
                        temp_details[i+8].set(temps[i+1]);
                        print("Temp sensor #" + str(i+8) + ": " + temp_details[i+8].get() + "C");
                
        elif id >= 100 and id < 140:
                pack_num = int(id%100/10);
                print(int(id%10));
                if id%10%2 is 0:
                        print(log_msg[2:timestamp_end] + "[REQUEST] " + "Pack " + str(pack_num) + (" Status" if id%10 == 0 else(" Voltages of cells 0-5" if id%10 == 2 else " Voltages of cells of 6-11")) + " Requested");

                elif id%10 is 1:
                        values = log_msg.split(" ");

                        for i in range(12):
                                pack_details[pack_num][i][1].set("OK" if int(values[i + 1]) == 0 else ("LOW" if int(int(values[i + 1])/100) == 1 else "HIGH"));
                                pack_details[pack_num][i][2].set("OK" if int(values[i + 1])%10 == 0 else "SHUN");
                        pack_details[pack_num][12][1].set("OK" if int(values[13]) == 0 else "LOW" if int(values[13]) == 1 else "HIGH");
                        pack_details[pack_num][13][1].set("OK" if int(values[14][0]) == 0 else "LOW" if int(values[14][0]) == 2 else "HIGH");

                elif id%10 in [3,5]:
                        half = 0 if id%10 == 3 else 1;
                        values = log_msg.split(" ");
                        for i in range(6):
                                pack_details[pack_num][i + half*6][0].set(values[i + 1]);
                        pack_details[pack_num][12 + half][0].set(values[7][:len(values[7]) - 5]);

def wait():
        log_msg = str(ser.readline());
        print(log_msg);

        if log_msg.find("]{") > -1:
                update(log_msg);
   
        root.after(100, wait);

root.after(10, wait);
root.mainloop();
