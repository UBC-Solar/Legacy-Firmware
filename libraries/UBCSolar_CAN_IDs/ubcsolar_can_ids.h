/* RULES
 * 1. Keep the IDs in order from lowest to highest
 * 2. If you use a range of IDs, put the last ID you use in this file for informational purposes.
 */

#ifndef UBCSOLAR_CAN_IDS //include only once
#define UBCSOLAR_CAN_IDS

#define CAN_ID_BRAKE 0
#define CAN_ID_HEARTBEAT 6

#define CAN_ID_AUX_BMS 7

#define CAN_ID_MPPT_CURRENT_WARNING 8
#define CAN_ID_MPPT_TEMP_WARNING 9

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


#define CAN_ID_MPPT_CURRENT_SENSOR_1 202
#define CAN_ID_MPPT_CURRENT_SENSOR_2 201

#define CAN_ID_MPPT_TEMP_SENSOR_1 199
#define CAN_ID_MPPT_TEMP_SENSOR_2 198
#define CAN_ID_MPPT_TEMP_SENSOR_3 197

#endif
