#ifndef UBCSOLAR_CAN //include only once
#define UBCSOLAR_CAN

#include <Arduino.h>
#include <stdint.h>
#include <mcp_can.h>
#include "ubcsolar_can_ids.h"

// a generic message
class Message {
  public:
    // Receive buffer fields
    uint32_t _frameId;
    uint8_t _msgLength;
    uint8_t _buffer[8];
};

// all known information about driver interface and status
class HeartbeatMessage : Message {
  private:
    // byte indices in payload
    static const uint8_t FORWARD_THROTTLE_INDEX = 0;
    static const uint8_t REVERSE_THROTTLE_INDEX = 1;
    static const uint8_t DIRECTION_STATUS_INDEX = 2;
    static const uint8_t INTERFACE_STATUS_INDEX = 3;
    // bitmasks for status indices
    static const uint8_t RIGHT_SIGNAL_MASK = 0x1;
    static const uint8_t LEFT_SIGNAL_MASK = 0x1 << 1;

  public:
    static const uint8_t CAN_ID = CAN_ID_HEARTBEAT;
    static const uint32_t HEARTBEAT_PERIOD = 100; // milliseconds

    enum class Direction : uint8_t {
      forward = 0,
      reverse = 1,
      neutral = 2,
    };

    enum class Signal : uint8_t {
      right = 1,
      left = 2,
      hazard = 3,
    };

    uint8_t getForwardThrottle();
    uint8_t getRegen();
    Direction getThrottleDirection();
    Signal getSignals();
};

// sent by nodes that do not receive heartbeats, for logging
class CommErrorMessage : Message {
  private:
    // byte indices in payload
    static const uint8_t SYSTEM_ID_INDEX = 0;
    static const uint8_t TIMESTAMP_INDEX = 1;

  public:
    static const uint8_t CAN_ID = CAN_ID_COMM_ERROR;

    enum class SystemId : uint8_t {
      handbrake = 0,
      commander = 1,
      motorControl = 2,
      // TODO more systems
    };

    SystemId getSystemId();
    uint32_t getTimestamp();
};

// signals application of the handbrake
class HandbrakeMessage : Message {
  public:
    static const uint8_t CAN_ID = CAN_ID_HANDBRAKE;

    // TODO
};

// encoder output updates for speed measurement
class EncoderMessage : Message {
  public:
    static const uint8_t CAN_ID = CAN_ID_SPEED;

    // TODO

};

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
   * @param message A storage class for writing the message if present
   * @return True if successful, false otherwise
   */
  boolean checkMessage(Message& message);

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

  boolean shouldLog();
};


#endif
