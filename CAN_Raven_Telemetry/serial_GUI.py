import serial
import time
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

var= {"time" : StringVar(), "timer" : StringVar(), "brake" : StringVar(), "hazard" : StringVar(), "velocity" : StringVar(),\
             "left" : StringVar(), "right" : StringVar(),\
             "accel" : StringVar(), "regen" : StringVar(), "dir" : StringVar(),\
             "status": StringVar(), "err" : StringVar(),\
             "soc" : StringVar(), "volt" : StringVar(), "aux volt" : StringVar(),\
             "current" : (StringVar(), StringVar(), StringVar(),), "temp" : (StringVar(), StringVar(), StringVar(),),\
      "pack" : [], "mppt curr" : [], "mppt temp" : []};

labels = {"battery" : [], "temp" : []};

last_msg = time.time();

frame = Frame(root);
frame.grid(row = 0);

var["time"].set("N/A");
var["timer"].set("N/A");

Label(frame, text = "Last CAN message: ", font = (None, 10, "bold",), width = 15, anchor = E, bg = "gray85").grid(row = 0, column = 0, sticky = E);
Label(frame, textvariable = var["time"], font = (None, 10,), width = 10, anchor = W, bg = "gray85").grid(row = 0, column = 1);
labels["timer"] = Label(frame, textvariable = var["timer"], font = (None, 10,), width = 25, anchor = W, bg = "gray85");
labels["timer"].grid(row = 0, column = 2);


var["left"].set("OFF");
var["right"].set("OFF");

Label(frame, text = "Left Signal: " , font = (None, 10, "bold",), width = 25, anchor = E, bg = "gray85").grid(row = 0, column = 3);
Label(frame, textvariable = var["left"], font = (None, 10,), width = 25, anchor = W, bg = "gray85").grid(row = 0, column = 4);

Label(frame, text = "Right Signal: ", font = (None, 10, "bold",), width = 25, anchor = E, bg = "gray85").grid(row = 0, column = 5);
Label(frame, textvariable = var["right"], font = (None, 10,) , width = 25, anchor = W, bg = "gray85").grid(row = 0, column = 6);

var["accel"].set("N/A");
var["regen"].set("N/A");
var["dir"].set("FORWARD");

frame = Frame(root);
frame.grid(row = 2, column = 0);

Label(frame, text = "Acceleration: ", font = (None, 10, "bold",), width = 25, anchor = E).grid(row = 0, column = 0);
Label(frame, textvariable = var["accel"], font = (None, 10, ), width = 25, anchor = W).grid(row = 0, column = 1);

Label(frame, text = "Regen: ", font = (None, 10, "bold",), width = 25, anchor = E).grid(row = 0, column = 2);
Label(frame, textvariable = var["regen"], font = (None, 10,), width = 25, anchor = W).grid(row = 0, column = 3);

Label(frame, text = "Direction: ", font = (None, 10, "bold",), width = 25, anchor = E).grid(row = 0, column = 4);
Label(frame, textvariable = var["dir"], font = (None, 10,), width = 25, anchor = W).grid(row = 0, column = 5);

var["status"].set("N/A");
var["err"].set(ERRORS[0]);

frame = Frame(root);
frame.grid(row = 3, column = 0);
Label(frame, text = "Status: ",font = (None, 10, "bold",), width = 50, anchor = E, bg = "gray85").grid(row = 0, column = 0);
Label(frame, textvariable = var["status"], font = (None, 10,), width = 25, anchor = W, bg = "gray85").grid(row = 0, column = 1);

Label(frame, text = "Error: ", font = (None, 10, "bold"), width = 25, anchor = E, bg = "gray85").grid(row = 0, column = 2);
labels["err"] = Label(frame, textvariable =var["err"], font = (None, 10,), width = 50, anchor = W, bg = "gray85");
labels["err"].grid(row = 0, column = 3);

var["soc"].set("N/A");
var["volt"].set("N/A");
var["aux volt"].set("N/A");

var["brake"].set("OFF");
var["hazard"].set("OFF");
var["velocity"].set("N/A");

var["current"][0].set("N/A");
var["current"][1].set("N/A");
var["current"][2].set("N/A");
bmsMainCurrentPeak = None;

def bmsMainCurrentReset():
        global bmsMainCurrentPeak;
        bmsMainCurrentPeak = None;
        var["current"][1].set("N/A");
        var["current"][2].set("N/A");


var["temp"][0].set("N/A");
var["temp"][1].set("N/A");
var["temp"][2].set("N/A");
bmsMainTempPeak = None;

def bmsMainTempReset():
        global bmsMainTempPeak;
        bmsMainTempPeak = None;
        var["temp"][1].set("N/A");
        var["temp"][2].set("N/A");

frame = Frame(root);
frame.grid(row = 4);

subframe = Frame(frame);
subframe.grid(row = 0, column = 0, columnspan = 2);
Label(subframe, text = "State of Charge: (%) ", font = (None, 10, "bold",), width = 20).grid(row = 0, column = 0);
Label(subframe, textvariable = var["soc"], font = (None, 15,), width = 20, height = 3).grid(row = 1, column = 0);

Label(subframe, text = "Voltage: (V)", font = (None, 10, "bold",), width = 20).grid(row = 0, column = 1);
Label(subframe, textvariable = var["volt"], font = (None, 15,), width = 20, height = 3).grid(row = 1, column = 1);

Label(subframe, text = "Aux Voltage: (V)", font = (None, 10, "bold",), width = 20).grid(row = 0, column = 2);
Label(subframe, textvariable = var["aux volt"], font = (None, 15,), width = 20, height = 3).grid(row = 1, column = 2);

Label(subframe, text = "Brake: " , font = (None, 10, "bold",), width = 20).grid(row = 2, column = 0);
Label(subframe, textvariable = var["brake"], font = (None, 15,), width = 10, height = 3).grid(row = 3, column = 0);

Label(subframe, text = "Hazard: " , font = (None, 10, "bold",), width = 25).grid(row = 2, column = 1);
Label(subframe, textvariable = var["hazard"], font = (None, 15,),  width = 25, height = 3).grid(row = 3, column = 1);

Label(subframe, text = "Velocity: " , font = (None, 10, "bold",), width = 25).grid(row = 2, column = 2);
Label(subframe, textvariable = var["velocity"], font = (None, 15,),  width = 25, height = 3).grid(row = 3, column = 2);

subframe = Frame(frame);
subframe.grid(row = 1, column = 0, sticky = N);
subsubframe = Frame(subframe);
subsubframe.grid(row = 0, pady = 60);

Label(subsubframe, text = "Current: (A)", font = (None, 10,"bold",), width = 10).grid(row = 0, column = 0);
Label(subsubframe, textvariable = var["current"][0], font = (None, 15,), width = 10, height = 3).grid(row = 1, column = 0, rowspan = 4);

Label(subsubframe, text = "Peak:", font = (None, 10, "bold",), width = 10).grid(row = 0, column = 1);
Label(subsubframe, textvariable = var["current"][1], font = (None, 10,), width = 10).grid(row = 1, column = 1);

Label(subsubframe, text = "Peak Time:", font = (None, 10,"bold",), width = 10).grid(row = 2, column = 1);
Label(subsubframe, textvariable = var["current"][2], font = (None, 10,), width = 10).grid(row = 3, column = 1);
Button(subsubframe, text = "RESET", command = bmsMainCurrentReset).grid(row = 4, column = 1);

subsubframe = Frame(subframe);
subsubframe.grid(row = 1, pady = 20);

Label(subsubframe, text = "Temp: (Cel.)", font = (None, 10, "bold"), width = 10).grid(row = 0, column = 0);
Label(subsubframe, textvariable = var["temp"][0], font = (None, 15,), width = 10, height = 3).grid(row = 1, column = 0, rowspan = 4);

Label(subsubframe, text = "Peak:", font = (None, 10,"bold",), width = 10).grid(row = 0, column = 1);
Label(subsubframe, textvariable = var["temp"][1], font = (None, 10,), width = 10).grid(row = 1, column = 1);

Label(subsubframe, text = "Peak Time:", font = (None, 10,"bold",), width = 10).grid(row = 2, column = 1);
Label(subsubframe, textvariable = var["temp"][2], font = (None, 10,), width = 10).grid(row = 3, column = 1);
Button(subsubframe, text = "RESET", command = bmsMainTempReset).grid(row = 4, column = 1);

subframe = Frame(frame);
subframe.grid(column = 2, rowspan = 4, row = 0);

def reset_bms_temp(pack_num, temp):
        row_num = 2 if pack_num is 3 else 3;
        var["pack"][pack_num][row_num + temp][2].set("N/A");
        var["pack"][pack_num][row_num + temp][3].set("N/A");

for i in range(4):
        subsubframe = Frame(subframe, width = 50);
        subsubframe.grid(row = i*2);
        
        Label(subsubframe, text = "Pack " + str(i + 1) + ":", font = (None, 12, "bold"), width = 50, anchor = W).grid(row = 0, columnspan = 4);
        Label(subsubframe, text = "Cell: " , font = (None, 10, "bold",), width = 5).grid(row = 1, column = 0);
        Label(subsubframe, text = "Voltage: (V)" , font = (None, 10, "bold",), width = 10).grid(row = 1, column = 1);
        Label(subsubframe, text = "Volt warning: " , font = (None, 10, "bold",), width = 10).grid(row = 1, column = 2);
        Label(subsubframe, text = "Shun warning: " , font = (None, 10, "bold",), width = 12).grid(row = 1, column = 3);
        var["pack"].append([]);
        labels["battery"].append([]);
        labels["temp"].append([]);
        row_count = 2 if i is 3 else 3;

        for j in range(row_count):
                Label(subsubframe, text = str(j) + ":", font = (None, 8, "bold",), width = 12).grid(row = j + 2, column = 0);
                var["pack"][i].append((StringVar(), StringVar(), StringVar(),));
                var["pack"][i][j][0].set("N/A");
                var["pack"][i][j][1].set("OK");
                var["pack"][i][j][2].set("OK");
                Label(subsubframe, textvariable = var["pack"][i][j][0], font = (None, 10,), width = 14).grid(row = j + 2, column = 1);
                labels["battery"][i].append((Label(subsubframe, textvariable = var["pack"][i][j][1] , font = (None, 10,), width = 12), \
                                    Label(subsubframe,textvariable = var["pack"][i][j][2] , font = (None, 10,), width = 12),));
                labels["battery"][i][j][0].grid(row = j + 2, column = 2);
                labels["battery"][i][j][1].grid(row = j + 2, column = 3);

        subsubframe = Frame(subframe);
        subsubframe.grid(row = i*2 + 1);
        Label(subsubframe, text = "Temperature:  ", font = (None, 8, "bold",), width = 11).grid(row = 0, column = 1);
        Label(subsubframe, text = "Warning :", font = (None, 8, "bold",), width = 10).grid(row = 0, column = 2);
        Label(subsubframe, text = "Peak :", font = (None, 8, "bold",), width = 10).grid(row = 0, column = 3);
        Label(subsubframe, text = "Time of Peak :", font = (None, 8, "bold",), width = 10).grid(row = 0, column = 4);
        Label(subsubframe, text = "Reset Peak:", font = (None, 8, "bold",), width = 10).grid(row = 0, column = 5);

        for k in range(2):
                Label(subsubframe, text = "Temperature " + str(k) + ": ", font = (None, 8, "bold",), width = 12, height = 1).grid(row = k + 1);
                var["pack"][i].append((StringVar(), StringVar(), StringVar(), StringVar()));
                var["pack"][i][k + row_count][0].set("N/A");
                var["pack"][i][k + row_count][1].set("OK");
                var["pack"][i][k + row_count][2].set("N/A");
                var["pack"][i][k + row_count][3].set("N/A");
                Label(subsubframe, textvariable = var["pack"][i][row_count + k][0], font = (None, 8,), width = 10, height = 1).grid(row = k + 1, column = 1);
                labels["temp"][i].append((Label(subsubframe, textvariable = var["pack"][i][row_count + k][1], font = (None, 8,), width = 10, height = 1), \
                                       Label(subsubframe, textvariable = var["pack"][i][row_count + k][2], font = (None, 8,), width = 10, height = 1),));
                Label(subsubframe, textvariable = var["pack"][i][row_count + k][3], font = (None, 8,), width = 10, height = 1).grid(row = k + 1, column = 4);
                labels["temp"][i][k][0].grid(row = k + 1, column = 2);
                labels["temp"][i][k][1].grid(row = k + 1, column = 3);
                Button(subsubframe, text = "RESET", width = 10, command = lambda x = i, y = k:reset_bms_temp(x, y), height = 1).grid(row = k + 1, column = 5);

                
subframe = Frame(frame, width = 45);
subframe.grid(row= 1, column = 1, sticky = N);
Label(subframe, text = "MPPT info", font = (None, 15,"bold",)).grid(row = 0);

subsubframe = Frame(subframe , width = 45);
subsubframe.grid(row = 1, column = 0, sticky = N);
Label(subsubframe, text = "Current (A)", font = (None, 10, "bold",), width = 10).grid(row = 0, column = 1);
Label(subsubframe, text = "Peak", font = (None, 10, "bold",), width = 10).grid(row = 0, column = 2);
Label(subsubframe, text = "Peak Time", font = (None, 10, "bold",), width = 10).grid(row = 0, column = 3);
Label(subsubframe, text = "Reset", font = (None, 10, "bold",), width = 5).grid(row = 0, column = 4);

mppt_peak_currs = [0,0,0,0,0,0];
mppt_peak_temps = [0,0,0,0,0,0,0,0,0,0];
def reset_mppt_peak(key, index):
        print(key);
        print(index);
        var[key][index][2].set("N/A");
        var[key][index][1].set("N/A");
        if key == "mppt curr":
                mppt_peak_currs[index] = 0;
        if key == "mppt temp":
                mppt_peak_temps[index] = 0;

for i in range(6):
        Label(subsubframe, text = str(i) + ":", font = (None, 10,), width = 5).grid(row = i + 1, column = 0);
        var["mppt curr"].append((StringVar(), StringVar(), StringVar(),));
        Button(subsubframe, text = "RESET", command = lambda x = i:reset_mppt_peak("mppt curr" , x)).grid(row = i + 1, column = 4);

        for j in range(3):
                var["mppt curr"][i][j].set("N/A");
                Label(subsubframe, textvariable = var["mppt curr"][i][j], font = (None, 10,), width = 10).grid(row = i + 1, column = j + 1);

subsubframe = Frame(subframe, width = 45);
subsubframe.grid(row = 2, column = 0, pady = 20);

Label(subsubframe, text = "Temp (C)", font = (None, 10, "bold",), width = 10).grid(row = 0, column = 1);
for i in range(10):
        Label(subsubframe, text = str(i) + ":", font = (None, 10,), width = 5).grid(row = i + 1, column = 0);
        var["mppt temp"].append((StringVar(), StringVar(), StringVar(),));
        Button(subsubframe, text = "RESET", command = lambda x = i: reset_mppt_peak("mppt temp", x)).grid(row = i + 1, column = 4);

        for j in range(3):
                var["mppt temp"][i][j].set("N/A");
                Label(subsubframe, textvariable = var["mppt temp"][i][j], font = (None, 10,), width = 10).grid(row = i + 1, column = j + 1);

                
def update(log_msg):
        global last_msg;
        global bmsMainCurrentPeak;
        global bmsMainTempPeak;
        timestamp_end = log_msg.find("]")+1;
        data_start = log_msg.find("}")+2;
        id = int(log_msg[log_msg.find("{")+1:log_msg.find("}")]);
        timestamp = log_msg[2:timestamp_end];
        var["time"].set(timestamp);
        
        if id == BRAKE_SIGNAL:
                var["brake"].set("ON" if int(log_msg[data_start]) else "OFF");
                print(log_msg[2:timestamp_end] + "[UPDATE] Brake: " + brake.get());

        elif id == HEARTBEAT_SIGNAL:
                var["accel"].set(str(float(log_msg[log_msg.find("A")+1:log_msg.find("R")]) / 255.0 * 100.0));
                var["regen"].set(str(float(log_msg[log_msg.find("R")+1:log_msg.find("D")]) / 255.0 * 100.0));
                var["dir"].set("FORWARD" if int(log_msg[log_msg.find("D")+1:log_msg.find("S")]) else "REVERSE")

                signals = "{0:08b}".format(int(log_msg[log_msg.find("S")+1:log_msg.find("E")]));
                var["hazard"].set("ON" if int(signals[3]) else "OFF");
                var["left"].set("ON" if int(signals[7]) else "OFF");
                var["right"].set("ON" if int(signals[6]) else "OFF");
                print(log_msg[2:timestamp_end] + "[HEARTBEAT] ");
                print("Acceleration%: " + var["accel"].get() + "\tRegen%: " + var["regen"].get() + "\tDirection: " + var["dir"].get());
                print("Left signal: " + var["left"].get() + "\tRight signal: " + var["right"].get() + "\tHazard: " + var["hazard"].get());

        elif id == BMS_CORE_STATUS:
                values = log_msg.split("} ")[1].split(" ");
                var["status"].set(STATES[int(values[0])%4]); #mod 4 is there just for testing. remove after
                var["err"].set(ERRORS[int(values[1])]);
                var["soc"].set(int(values[2]));
                var["volt"].set(int(values[3]));
                currentValue = int(values[4]);
                var["current"][0].set(currentValue);
                if(bmsMainCurrentPeak is None or currentValue > bmsMainCurrentPeak):
                        var["current"][1].set(currentValue);
                        bmsMainCurrentPeak = currentValue;
                        var["current"][2].set(timestamp);
                var["aux volt"].set(float(values[5]));
                tempValue = int(values[6][:len(values[6]) - 5])
                var["temp"][0].set(tempValue);
                if(bmsMainTempPeak is None or tempValue > bmsMainTempPeak):
                        var["temp"][1].set(tempValue);
                        bmsMainTempPeak = tempValue;
                        var["temp"][2].set(timestamp);
                labels["err"].config(fg = ("GREEN" if var["err"].get() == "NONE" else "red"));
                
                print(log_msg[2:timestamp_end] + ("[WARNING]" if (int(values[1]) in [2,4,6,9]) else "[ERROR]" if int(values[1]) is 15 else "[UPDATE]") +\
                      " Status: " + var["status"].get() + " Error: " + ERRORS[int(values[1])] + \
                      " SoC: " + values[2] + "% Voltage: " + values[3] + "V Current: " + values[4] +\
                        "A Aux Voltage: " + var["aux volt"].get() + "V Temperature: " + \
                        var["temp"][0].get());

        elif id == CURRENT_SIGNAL_1: #CURRENT_SIGNAL_X and TEMP_SIGNAL_X refer to MPPT
                currents = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT CURRENT1] ");
                for i in range(4):
                        thisCurrent = currents[i+1];
                        var["mppt curr"][i][0].set(thisCurrent);
                        if float(thisCurrent) > mppt_peak_currs[i]:
                                var["mppt curr"][i][1].set(thisCurrent);
                                var["mppt curr"][i][2].set(timestamp);
                                mppt_peak_currs[i] = float(thisCurrent);
                        
                        print("Current sensor #" + str(i) + ": " + var["mppt curr"][i][0].get() + "A");

        elif id == CURRENT_SIGNAL_2:
                currents = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT CURRENT2] ");
                for i in range(2):
                        thisCurrent = currents[i+1];
                        var["mppt curr"][i+4][0].set(thisCurrent);
                        if float(thisCurrent) > mppt_peak_currs[i+4]:
                                var["mppt curr"][i + 4][1].set(thisCurrent);
                                var["mppt curr"][i + 4][2].set(timestamp);
                                mppt_peak_currs[i+4] = float(thisCurrent);
                        print("Current sensor #" + str(i+4) + ": " + var["mppt curr"][i+4][0].get() + "A");

        elif id == TEMP_SIGNAL_1:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP1] ");
                for i in range(4):
                        thisTemp = temps[i+1];
                        var["mppt temp"][i][0].set(thisTemp);
                        if float(thisTemp) > mppt_peak_temps[i]:
                                var["mppt temp"][i][1].set(thisTemp);
                                var["mppt temp"][i][2].set(timestamp);
                                mppt_peak_temps[i] = float(thisTemp);
                        print("Temp sensor #" + str(i) + ": " + var["mppt temp"][i][0].get() + "C");

        elif id == TEMP_SIGNAL_2:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP2] ");
                for i in range(4):
                        thisTemp = temps[i+1];
                        var["mppt temp"][i+4][0].set(thisTemp);
                        if float(thisTemp) > mppt_peak_temps[i+4]:
                                var["mppt temp"][i+4][1].set(thisTemp);
                                var["mppt temp"][i+4][2].set(timestamp);
                                mppt_peak_temps[i+4] = float(thisTemp);
                        print("Temp sensor #" + str(i+4) + ": " + var["mppt temp"][i+4][0].get() + "C");

        elif id == TEMP_SIGNAL_3:
                temps = log_msg.split();
                print(log_msg[2:timestamp_end] + "[MPPT TEMP3] ");
                for i in range(2):
                        thisTemp = temps[i+1];
                        var["mppt temp"][i+8][0].set(thisTemp);
                        if float(thisTemp) > mppt_peak_temps[i+8]:
                                var["mppt temp"][i+8][1].set(thisTemp);
                                var["mppt temp"][i+8][2].set(timestamp);
                                mppt_peak_temps[i+8] = float(thisTemp);
                        print("Temp sensor #" + str(i+8) + ": " + var["mppt temp"][i+8][0].get() + "C");
                
        elif id >= 100 and id < 140:
                pack_num = int(id%100/10);
                row_count = 2 if pack_num is 3 else 3;
                if id%10%2 is 0:
                        print(log_msg[2:timestamp_end] + "[REQUEST] " + "Pack " + str(pack_num) + (" Status" if id%10 == 0 else(" Cell Voltages and Temperature 0" if id%10 == 2 else " Temperature 1")) + " Requested");

                elif id%10 is 1:
                        values = log_msg.split(" ");
                        print(log_msg[2:timestamp_end] + "[BMS UPDATE] Pack " + str(pack_num), end = "  ");

                        for i in range(row_count):
                                var["pack"][pack_num][i][1].set("LOW" if ((int(values[1]) >> i)%2) == 1 else ("HIGH" if ((int(values[2]) >> i)%2) == 1 else "OK"));
                                labels["battery"][pack_num][i][0].config(fg = ("Green" if var["pack"][pack_num][i][1].get() == "OK" else "Red"));
                                var["pack"][pack_num][i][2].set("OK" if ((int(values[3]) >> i)%2) == 0 else "SHUN");
                                labels["battery"][pack_num][i][1].config(fg = ("Green" if var["pack"][pack_num][i][2].get() == "OK" else "Red"));
                                print("Cell " + str(i) + ": Voltage: " + var["pack"][pack_num][i][1].get() + " Shun? " + var["pack"][pack_num][i][2].get(), end = " ");
                                
                        var["pack"][pack_num][row_count][1].set("OK" if int(values[4]) == 0 else "LOW" if int(values[4]) == 1 else "HIGH");
                        labels["temp"][pack_num][0][0].config(fg = ("Green" if var["pack"][pack_num][row_count][1].get() == "OK" else "Red"));
                        var["pack"][pack_num][row_count + 1][1].set("OK" if int(values[5][0]) == 0 else "LOW" if int(values[5][0]) == 4 else "HIGH");
                        labels["temp"][pack_num][1][0].config(fg = ("Green" if var["pack"][pack_num][row_count + 1][1].get() == "OK" else "Red"));
                        print("Temperature 1: " + var["pack"][pack_num][row_count][1].get() + "\tTemperature 2: " + var["pack"][pack_num][row_count + 1][1].get());

                elif id%10 is 3:
                        print(log_msg[2:timestamp_end] + "[BMS UPDATE] Pack " + str(pack_num + 1) + "Cell Voltages: ", end = "");
                        values = log_msg.split(" ");
                        for i in range(row_count):
                                var["pack"][pack_num][i][0].set(values[i + 1]);
                                print(values[i + 1] + "V ", end = "");
                        var["pack"][pack_num][row_count][0].set(values[4][:len(values[4]) - 5]);
                        if var["pack"][pack_num][row_count][2].get() == "N/A" or int(var["pack"][pack_num][row_count][0].get()) > int(var["pack"][pack_num][row_count][2].get()):
                              var["pack"][pack_num][row_count][2].set(var["pack"][pack_num][row_count][0].get());
                              var["pack"][pack_num][row_count][3].set(log_msg[2:timestamp_end]);
                        print("Temperature 0:" + var["pack"][pack_num][row_count][0].get());
                        
                elif id%10 is 5:
                        value = log_msg.split(" ")[1];
                        print(len(value));
                        var["pack"][pack_num][row_count + 1][0].set(value[:len(value) - 5]);
                        if var["pack"][pack_num][row_count + 1][2].get() == "N/A" or int(var["pack"][pack_num][row_count + 1][0].get()) > int(var["pack"][pack_num][row_count + 1][2].get()):
                              var["pack"][pack_num][row_count + 1][2].set(var["pack"][pack_num][row_count + 1][0].get());
                              var["pack"][pack_num][row_count + 1][3].set(log_msg[2:timestamp_end]);
                        
                        print(log_msg[2:timestamp_end] + "[BMS UPDATE] Pack " + str(pack_num + 1) + " Temperature 1:" + var["pack"][pack_num][row_count + 1][0].get());
                        
        last_msg = time.time();

def wait():
        global last_msg;
        if ser.inWaiting():
                log_msg = str(ser.readline());
                print(log_msg);
                if log_msg.find("]{") > -1:
                        try:
                         update(log_msg);
                        except:
                                print("CAN message ERROR");
                
        var["timer"].set(str(int(time.time() - last_msg)) + " secs since last msg");
        labels["timer"].config(fg = "black" if (time.time() - last_msg) < 5 else "red");
        root.after(100, wait);

root.after(10, wait);
root.mainloop();
