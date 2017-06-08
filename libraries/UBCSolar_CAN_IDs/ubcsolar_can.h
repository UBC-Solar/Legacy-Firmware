#ifndef UBCSOLAR_CAN //include only once
#define UBCSOLAR_CAN

#include "ubcsolar_can_ids.h"

#include <stdint.h>

class MCP_CAN;

namespace comm {

  // all known information about driver interface and status
  class Heartbeat {
    private:
      static const uint8_t CAN_ID = CAN_ID_HEARTBEAT;
      static const uint32_t HEARTBEAT_PERIOD = 100; // milliseconds
      // byte indices in payload
      static const uint8_t FORWARD_THROTTLE_INDEX = 0;
      static const uint8_t REVERSE_THROTTLE_INDEX = 1;
      static const uint8_t DIRECTION_STATUS_INDEX = 2;
      static const uint8_t INTERFACE_STATUS_INDEX = 3;
      // bitmasks for status indices
      static const uint8_t RIGHT_SIGNAL_MASK = 0x1;
      static const uint8_t LEFT_SIGNAL_MASK = 0x2;

    public:
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

      static uint8_t getId(const uint8_t* payload);
      static uint32_t getPeriod(const uint8_t* payload);
      static uint8_t getForwardThrottle(const uint8_t* payload);
      static uint8_t getRegen(const uint8_t* payload);
      static Direction getThrottleDirection(const uint8_t* payload);
      static Signal getSignals(const uint8_t* payload);
  };

  // sent by nodes that do receive heartbeats, for logging
  class CommError {
    private:
      static const uint8_t CAN_ID = CAN_ID_COMM_ERROR;
      // byte indices in payload
      static const uint8_t SYSTEM_ID_INDEX = 0;
      static const uint8_t TIMESTAMP_INDEX = 1;

      static const uint8_t TIMESTAMP_BYTES = 7;

    public:
      enum class SystemId : uint8_t {
        handbrake = 0,
        commander = 1,
        motorControl = 2,
        // more TBD
      };

      static SystemId getSystemId(const uint8_t* payload);
      static uint32_t getTimestamp(const uint8_t* payload);
  };

  // signals application of the handbrake
  class Handbrake{
    private:
      static const uint8_t CAN_ID = CAN_ID_HANDBRAKE;

      // TODO
  };

  // encoder output updates for speed measurement
  class Speed {
    private:
      static const uint8_t CAN_ID = CAN_ID_SPEED;

      // TODO

  };

  // initialize the CAN shield
  MCP_CAN initCAN(uint8_t pin1, uint8_t pin2);

}

#endif
