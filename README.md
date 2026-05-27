# Arduino NRF24 RTS/CTS Network

Distributed communication system using Arduino and NRF24L01 modules with a custom RTS/CTS handshake protocol inspired by the MACA 4-way handshake protocol.

---

## Overview

This project implements a simple distributed wireless network composed of:

- Sensor Node
- Gateway Node
- Server Node

Communication is performed using NRF24L01 radio modules and a custom packet protocol based on the MACA (Multiple Access with Collision Avoidance) 4-way handshake:

- RTS (Request To Send)
- CTS (Clear To Send)
- DATA
- ACK

The goal is to provide reliable packet delivery between nodes without using the built-in auto acknowledgment from the RF24 library.

---

## Features

- MACA-inspired 4-way handshake protocol
- Custom RTS/CTS communication
- Multi-node architecture
- Presence sensor integration
- Gateway packet forwarding
- Manual ACK handling
- Debug messages via Serial Monitor
- Low-level packet management

---

## Hardware Requirements

- Arduino Uno/Nano
- NRF24L01 modules
- PIR motion sensor
- LED
- Jumper wires
- Breadboard

---

## Network Topology

```text
                  MACA 4-Way Handshake Protocol

┌────────────┐                               ┌────────────┐
│   Sensor   │                               │  Gateway   │
│    Node    │                               │    Node    │
└─────┬──────┘                               └─────┬──────┘
      │                                            │
      │ ---------------- RTS --------------------> │
      │                                            │
      │ <--------------- CTS --------------------- │
      │                                            │
      │ ---------------- DATA -------------------> │
      │                                            │
      │ <--------------- ACK --------------------- │
      │                                            │


                    Packet Forwarding to Server

┌────────────┐          ┌────────────┐          ┌────────────┐
│   Sensor   │          │  Gateway   │          │   Server   │
│    Node    │ -------> │    Node    │ -------> │    Node    │
└────────────┘          └────────────┘          └────────────┘
```

### Communication Steps

1. Sensor node detects movement
2. Sensor sends an RTS packet to the Gateway
3. Gateway responds with CTS
4. Sensor transmits DATA packet
5. Gateway confirms reception with ACK
6. Gateway forwards the received data to the Server node

---

## Packet Structure

```cpp
struct packet {
  enum packet_type type;
  int source_ID;
  int destination_ID;
  int data;
};
```

---

## Packet Types

| Type | Description |
|------|-------------|
| RTS  | Request to send |
| CTS  | Clear to send |
| DATA | Data transmission |
| ACK  | Acknowledgment |

---

## Node Configuration

Change the `ID` value before uploading the code to each Arduino board.  
You can also customize the pin definitions for LEDs, sensors, buzzers, or any other hardware components you want to use.
```cpp
#define ID 44
```

### Available IDs

| Node | ID |
|------|----|
| Gateway | 4 |
| Server | 1 |
| Sensor | 44 |

---

## Running the Project

Inside the `loop()` function, uncomment only the desired node behavior for each Arduino board.

### Sensor Node

```cpp
loop_sensor();
```

### Gateway Node

```cpp
loop_gateway();
```

### Server Node

```cpp
loop_server();
```

---

## RF24 Configuration

```cpp
radio.setPALevel(RF24_PA_MAX);
radio.setChannel(100);
radio.setPayloadSize(sizeof(struct packet));
radio.setAutoAck(false);
radio.setCRCLength(RF24_CRC_DISABLED);
radio.setDataRate(RF24_250KBPS);
```

---

## How It Works

1. Sensor detects movement
2. Sensor sends RTS packet to Gateway
3. Gateway replies with CTS
4. Sensor sends DATA packet
5. Gateway sends ACK
6. Gateway forwards data to Server using the same protocol

---

## Serial Output Example

```text
Movimento detectado
Sensor: Transmitindo RTS...
-> CTS Recebido com sucesso! Enviando DATA...
Sensor: Transação concluída! Dados entregues.
```

---


## License

MIT License
