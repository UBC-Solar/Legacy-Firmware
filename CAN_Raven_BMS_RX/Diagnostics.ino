#define ANSI_ESC 0x1B

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
  Serial.print("[2J");
  diag_cursorPosition(1,1);
}

void diag_help(void){
  diag_cursorPosition(20,1);
  Serial.println("0: Turn diagnostics off");
  Serial.println("1: BMS Core Status");
  Serial.println("2: BMS Cells Status");
  Serial.println("3: Other Status");
  Serial.write(ANSI_ESC);
  Serial.print("[?25l"); //hide cursor
}

void diag_getCmd(byte cmd){
  switch(cmd){
    case '0':
      diagnosticMode = DIAG_OFF;
      break;
    case '1':
      diagnosticMode = DIAG_BMS_CORE;
      diag_BMSCoreLabels();
      break;
    case '2':
      diagnosticMode = DIAG_BMS_CELLS;
      diag_BMSCellsLabels();
      break;
    case '3':
      diagnosticMode = DIAG_OTHERS;
      diag_othersLabels();
      break;
  }
}

void diag_BMSCoreLabels(void){
  diag_clearScreen();
  Serial.println("DIAGNOSTICS: BMS CORE STATUS");
  Serial.println("Error Code:");
  Serial.println("Status Code:");
  Serial.println("State of Charge (%):");
  Serial.println("Voltage (V):");
  Serial.println("Current (A):");
  Serial.println("Aux Voltage (V):");
  Serial.println("Temperature (C):");
  diag_help();
}

void diag_BMSCore(void){
  diag_cursorPosition(2, 41);
  Serial.print(bmsStatus.error);
  diag_cursorPosition(3, 41);
  Serial.print(bmsStatus.status);
  diag_cursorPosition(4, 41);
  Serial.print(bmsStatus.soc);
  diag_cursorPosition(5, 41);
  Serial.print(bmsStatus.voltage);
  diag_cursorPosition(6, 41);
  Serial.print(bmsStatus.current);
  diag_cursorPosition(7, 41);
  Serial.print(bmsStatus.aux_voltage);
  diag_cursorPosition(8, 41);
  Serial.print(bmsStatus.temperature);
}

void diag_BMSCellsLabels(void){
  diag_clearScreen();
  Serial.println("DIAGNOSTICS: BMS CELLS STATUS");
  //            1         11        21        31        41
  Serial.println("          PACK0     PACK1     PACK2     PACK3");
  for(int i=0; i<12; i++){
    Serial.print("CELL");
    Serial.println(i);
  }
  Serial.println();
  for(int i=0; i<2; i++){
    Serial.print("TEMP");
    Serial.println(i);
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
  Serial.println("DIAGNOSTICS: OTHERS");
  Serial.println("Brake:");
  Serial.println("");
  Serial.println("Motor Throttle:");
  Serial.println("Motor Regen:");
  Serial.println("Motor Direction:");
  Serial.println("");
  Serial.println("MPPT Current:");
  diag_help();
}

void diag_others(void){
  diag_cursorPosition(2,41);
  Serial.print(brake_on);
  diag_cursorPosition(4,41);
  Serial.print(target_throttle);
  diag_cursorPosition(5,41);
  Serial.print(target_regen);
  diag_cursorPosition(6,41);
  Serial.print(target_dir);
}

