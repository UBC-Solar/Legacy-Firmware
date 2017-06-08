/* RULES
 * 1. Keep the IDs in order from lowest to highest
 * 2. If you use a range of IDs, put the last ID you use in this file for informational purposes.
 */

#ifndef UBCSOLAR_CAN_IDS //include only once
#define UBCSOLAR_CAN_IDS

#define CAN_ID_BRAKE 0
#define CAN_ID_HAZARD 1
#define CAN_ID_MOTOR_CTRL 4
#define CAN_ID_SPEED_SENSOR 8
#define CAN_ID_SIGNAL_CTRL 9

#define CAN_ID_ZEVA_BMS_CORE_STATUS 10
#define CAN_ID_ZEVA_BMS_CORE_SET_STATE 11
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_WR1 12
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_RD1 13
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_WR2 14
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_RD2 15
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_WR3 16
#define CAN_ID_ZEVA_BMS_CORE_CONFIG_RD3 17
#define CAN_ID_ZEVA_BMS_CORE_SET_CELL_NUM 18
#define CAN_ID_ZEVA_BMS_CORE_ACK_ERROR 19
#define CAN_ID_ZEVA_BMS_CORE_RESET_SOC 20

#define CAN_ID_ZEVA_BMS_BASE 100
#define CAN_ID_ZEVA_BMS_END 139 //informational purpose only. not used in code

#endif