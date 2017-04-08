#ifndef TELEMETRY_CANDRIVER_H
#define TELEMETRY_CANDRIVER_H

#include <mcp_can.h>
#include "DataTypes.h"
#include "DataLogger.h"

/**
 * @brief Wrapper for MCP_CAN library.
 */
class CANDriver {
 public:
  /**
   * Creates a new CANDriver instance to parse and handle CAN messages.
   * CANDriver will attempt to read CAN messages from the supplied pin. If
   * that doesn't work, it will try the alternate pin.
   * @param pin The Arduino pin.
   * @param alternate Alternate pin.
   * @param baudrate The baudrate; see MCP_CAN library for definitions.
   */
  CANDriver(uint8_t pin=10, uint8_t alternate=9, uint8_t baudrate=CAN_125KBPS);

  /**
   * Reads a CAN message, if there is one.
   * @param logger The |DataLogger| that will handle the message.
   */
  void checkMessage(DataLogger* logger);

  /**
   * Initializes the CANDriver so that it can read messages.
   */
  void begin();

 private:
  // Initialization fields
  MCP_CAN _can;
  uint8_t _pin;
  uint8_t _alternate;
  uint8_t _baudrate;

  // Message receive buffer fields
  uint32_t _frameId;
  uint8_t _msgLength;
  uint8_t _message[8];

  boolean shouldLog();
};

#endif // TELEMETRY_CANDRIVER_H
