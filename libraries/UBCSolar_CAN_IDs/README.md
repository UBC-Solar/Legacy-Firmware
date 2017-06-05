# Overview of CAN Usage for Raven

## CAN Identifiers
- Keep the IDs in order from lowest to highest
- Keep an open mind about future needs for high priority numbers

### Current CAN IDs
#### Raven defined
- Handbrake 1
- Comm. Error 5
- Heartbeat 6
- Speed 8 (encoder not yet present)

#### Externally defined
- Zeva bms core status 10
- Zeva bms core set state 11
- Zeva bms core config wr1 12
- Zeva bms core config rd1 13
- Zeva bms core config wr2 14
- Zeva bms core config rd2 15
- Zeva bms core config wr3 16
- Zeva bms core config rd3 17
- Zeva bms core set cell num 18
- Zeva bms core ack error 19
- Zeva bms core reset soc 20
- Zeva bms base 100
- Zeva bms end 139 //informational purpose only. not used in code

### Deprecated IDs (for historical purposes)
- Brake 0
- Hazard 1
- Motor ctrl 4
- Speed sensor 8
- Signal ctrl 9

## Specific Messages
### Heartbeat Message
New system for CAN communication will involve sending out heartbeat messages
from the driver interface which will contain all known information for the
consumption of each subsystem in every message.

This means that CAN IDs will be used more sparingly and the driver
interface will send out a message at a minimum heartbeat frequency or faster.
Further, these messages will indicate the complete status of the driver
interface, allowing for more informative logging.

The driver interface will send a CAN message when either (1) the minimum
heartbeat frequency ticks, or (2) a significant change occurs on a sensor
in the interface. For example, consider three cases:

1. The forward throttle sensor value changes above a threshold value
  - a message is sent immediately
  - the heartbeat timer is reset
2. The forward throttle sensor value changes below a threshold value
  - no message is sent until the next heartbeat timer overflow
3. The driver toggles the right turn signal
  - a message is sent immediately
  - the heartbeat timer is reset

In all cases, the message sent contains the current value of all sensors on
the driver interface and can be interpreted by any CAN node.

#### Structure of the message
- Byte0: forward throttle value
- Byte1: reverse throttle value
- Byte2: throttle direction
- Byte3: driver interface status register
  - bit0: right turn signal status
  - bit1: left turn signal status
  - bit2-7: tbd
- Byte3-7: tbd

#### Behaviour
Lastly, reading the heartbeat is required to verify that communications
are functioning correctly. Any subsystem that does not receive a heartbeat
signal within two consecutive heartbeat periods should send error messages
that can be logged by the telemetry system. Structure TBD.

The heartbeat period should be defined based on the longest time period
we are comfortable risking not knowing about a communications system failure.
Higher values however will reduce the number of messages on the CAN lines
and also the number being processed, so we don't want to just set it
arbitrarily low and burn tons of network capacity and computation time.
Significant changes in the interface will trigger early message sending.

### Error Message
Any system that has not received heartbeat messages for more than two
heartbeat periods should send error messages out to indicate a communications
failure. If the failure is severe, it is possible that the system will not
be able to send the messages or that the telemetry node will not be able to
receive them for logging, but it is low effort and would be helpful in
debugging the communication system down the road.

#### Structure of the message
- Byte0: value to identify node
- Bytes1-4: seconds since last received heartbeat
  - I think 4 bytes is sufficient to not worry about overflow with millisecond-resolution
