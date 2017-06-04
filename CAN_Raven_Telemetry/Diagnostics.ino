#define ANSI_ESC 0x1B
void printONOFF(int input) {
  if (input) {
    Serial.print("ON");
  }
  else {
    Serial.print("OFF");
  }
}

void printBMSCoreStatus() {
  switch(bms_status.status) {
    case IDLE:
      Serial.print("IDLE");
      break;
    case PRECHARGING:
      Serial.print("PRECHARGING");
      break;
    case RUNNING:
      Serial.print("RUNNING");
      break;
   case CHARGING:
      Serial.print("CHARGING");
      break;
    case SETUP:
      Serial.print("SETUP");
      break;
    default:
      break;
  }
}

void printBMSCoreError() {
  switch(bms_status.error) {
	case NOERR:
		Serial.print("NONE");
		break;
    case CORRUPT:
		Serial.print("SETTINGS CORRUPTED");
		break;
	case OVERCURRENTWARNING:
		Serial.print("OVERCURRENT WARNING");
		break;
	case OVERCURRENTSD:
		Serial.print("OVERCURRENT SHUTDOWN");
		break;
	case LOWCELLWARNING:
		Serial.print("LOW CELL VOLTAGE WARNING");
		break;
	case BMSSD:
		Serial.print("LOW CELL VOLTAGE SHUTDOWN");
		break;
	case HIGHCELLWARNING:
		Serial.print("HIGH CELL VOLTAGE WARNING");
		break;
	case BMSENDEDCHARGE:
		Serial.print("HIGH CELL VOLTAGE SHUTDOWN" );
		break;
	case BMSOVERTEMP:
		Serial.print("BMS OVERTEMPERATURE");
		break;
	case LOWSOCWARNING:
		Serial.print("LOW STATE OF CHARGE WARNING");
		break;
	case OVERTEMPERATURE:
		Serial.print("TEMPERATURE EXCEEDED WARNING LEVEL");
		break;
	case CHASISLEAK:
		Serial.print("CHASSIS LEAKAGE");
		break;
	case LOW12V:
		Serial.print("AUX BATTERY VOLTAGE BELOW WARNING LEVEL");
		break;
	case PRECHARGEFAIL:
		Serial.print("PRECHARGE FAILED");
		break;
	case CONTRATORSWITCHERROR:
		Serial.print("CONTATOR SWITCH ERROR");
		break;
	case CANERROR:
		Serial.print("CANBUS COMMUNICATION ERROR");
		break;
  default:
    break;
  }
}


/*
void diag_cursorPosition(int row, int col){
  Serial.write(ANSI_ESC);
  Serial.write('[');
  Serial.print(row);
  Serial.write(';');
  Serial.print(col);
  Serial.write('H');
}

void diag_clearScreen(void){
  Serial.write(ANSI_ESC);
  Serial.print(F("[2J"));
  diag_cursorPosition(1,1);
}

void diag_help(void){
  diag_cursorPosition(20,1);
  Serial.println(F("0: Turn diagnostics off"));
  Serial.println(F("1: BMS Core Status"));
  Serial.println(F("2: BMS Cells Status"));
  Serial.println(F("3: Other Status"));
  Serial.println(F("4: Set RTC Time"));
  //Serial.write(ANSI_ESC);
  //Serial.print(F("[?25l")); //hide cursor
}

void diag_getCmd(byte cmd){
  switch(cmd){
    case '0':
      diagnosticMode = DIAG_OFF;
      break;
    case '1':
      diagnosticMode = DIAG_BMS_CORE;
      diag_BMSCoreLabels();
      diag_BMSCore();
      break;
    case '2':
      diagnosticMode = DIAG_BMS_CELLS;
      diag_BMSCellsLabels();
      diag_BMSCells();
      break;
    case '3':
      diagnosticMode = DIAG_OTHERS;
      diag_othersLabels();
      diag_others();
      break;
    case '4':
      diagnosticMode = DIAG_SETTIME;
      diag_setTimeLabels();
      break;
  }
}

void diag_BMSCoreLabels(void){
  diag_clearScreen();
  Serial.println(F("DIAGNOSTICS: BMS CORE STATUS"));
  Serial.println(F("Error Code:"));
  Serial.println(F("Status Code:"));
  Serial.println(F("State of Charge (%):"));
  Serial.println(F("Voltage (V):"));
  Serial.println(F("Current (A):"));
  Serial.println(F("Aux Voltage (V):"));
  Serial.println(F("Temperature (C):"));
  diag_help();
}

void diag_BMSCore(void){
  diag_cursorPosition(2, 41);
  Serial.print(bmsStatus.error);
  Serial.print(F("  "));
  diag_cursorPosition(3, 41);
  Serial.print(bmsStatus.status);
  Serial.print(F("  "));
  diag_cursorPosition(4, 41);
  Serial.print(bmsStatus.soc);
  Serial.print(F("  "));
  diag_cursorPosition(5, 41);
  Serial.print(bmsStatus.voltage);
  Serial.print(F("  "));
  diag_cursorPosition(6, 41);
  Serial.print(bmsStatus.current);
  Serial.print(F("  "));
  diag_cursorPosition(7, 41);
  Serial.print(bmsStatus.aux_voltage);
  Serial.print(F("  "));
  diag_cursorPosition(8, 41);
  Serial.print(bmsStatus.temperature);
  Serial.print(F("  "));
}

void diag_BMSCellsLabels(void){
  diag_clearScreen();
  Serial.println(F("DIAGNOSTICS: BMS CELLS STATUS"));
  //            1         11        21        31        41
  Serial.println(F("          PACK0     PACK1     PACK2     PACK3"));
  for(int i=0; i<12; i++){
    Serial.print(F("CELL"));
    Serial.println(i);
  }
  Serial.println();
  for(int i=0; i<2; i++){
    Serial.print(F("TEMP"));
    Serial.println(i);
    Serial.print("  ");
  }
  diag_help();
}

void diag_BMSCells(void){
  for(int i=0; i<4; i++){
    for(int j=0; j<12; j++){
      diag_cursorPosition(j+3, 10*i+11);
      Serial.print(cellVoltagesX100[i][j] / 100.0);
    }
  }
  for(int i=0; i<4; i++){
    for(int j=0; j<2; j++){
      diag_cursorPosition(j+16, 10*i+11);
      Serial.print(bmsTemperatures[i][j]);
    }
  }
}

void diag_othersLabels(void){
  diag_clearScreen();
  Serial.println(F("DIAGNOSTICS: OTHERS"));
  Serial.println(F("Brake:"));
  Serial.println("");
  Serial.println(F("Motor Throttle:"));
  Serial.println(F("Motor Regen:"));
  Serial.println(F("Motor Direction:"));
  Serial.println("");
  Serial.println(F("MPPT Current:"));
  Serial.println("");
  Serial.println(F("RTC Time:"));
  diag_help();
}

void diag_others(void){
  diag_cursorPosition(2,41);
  Serial.print(brake_on);
  diag_cursorPosition(4,41);
  Serial.print(target_throttle);
  Serial.print(F("  "));
  diag_cursorPosition(5,41);
  Serial.print(target_regen);
  Serial.print(F("  "));
  diag_cursorPosition(6,41);
  Serial.print(target_dir);
  diag_cursorPosition(10,41);
  struct datetime dt;
  ds1302_readtime(&dt);
  print_time(&dt);
}

void diag_setTimeLabels(void){
  diag_clearScreen();
  Serial.println(F("Enter Date/Time:"));
  Serial.println(F("YYMMDDHHMMSS"));
}

void diag_setTime(char c){
  static char dateStr[12];
  static int i;

  // ENTER key
  if(c == 0x0D){
    goto setTime_do;
  // ESC key
  }else if(c == 0x1B){
    goto setTime_exit;
  // BACKSPACE key
  }else if(c == 0x08 || c == 0x7F){
    if(i > 0){
      i--;
      Serial.print(F("\b \b"));
    }
  }else if(c >= '0' && c <= '9'){
    if(i < 12){
      dateStr[i] = c;
      i++;
      Serial.write(c);
    }
  }
  return;

setTime_do:
  if(i < 12)
    goto setTime_error;
    
  struct datetime dt;
  dt.year = (dateStr[0]-'0')*10 + (dateStr[1]-'0');
  dt.month = (dateStr[2]-'0')*10 + (dateStr[3]-'0');
  dt.day = (dateStr[4]-'0')*10 + (dateStr[5]-'0');
  dt.hour = (dateStr[6]-'0')*10 + (dateStr[7]-'0');
  dt.minute = (dateStr[8]-'0')*10 + (dateStr[9]-'0');
  dt.second = (dateStr[10]-'0')*10 + (dateStr[11]-'0');

  if(!validate_time(&dt))
    goto setTime_error;

  ds1302_writetime(&dt);
  Serial.println(F("Time Set."));
  
setTime_exit:
  i = 0;
  diagnosticMode = DIAG_OFF;
  lastPrintTime = millis();
  return;

setTime_error:
  Serial.println(F("INVALID DATE/TIME!"));
  goto setTime_exit;
}
*/






