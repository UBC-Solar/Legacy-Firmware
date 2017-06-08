#include "ubcsolar_can.h"

#include <mcp_can.h>

using namespace comm;

uint8_t Heartbeat::getId(const uint8_t* payload) {

}

uint32_t Heartbeat::getPeriod(const uint8_t* payload) {

}

uint8_t Heartbeat::getForwardThrottle(const uint8_t* payload) {

}

uint8_t Heartbeat::getRegen(const uint8_t* payload) {

}

Heartbeat::Direction Heartbeat::getThrottleDirection(const uint8_t* payload) {

}

Heartbeat::Signal Heartbeat::getSignals(const uint8_t* payload) {

}

CommError::SystemId getSystemId(const uint8_t* payload) {

}

uint32_t CommError::getTimestamp(const uint8_t* payload) {

}


MCP_CAN initCAN(uint8_t pin1, uint8_t pin2) {

}
