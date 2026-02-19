#include "comm_interface.h"
CommInterface CI;
void CommInterface::init() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2, true);
  Serial.println("Half-Duplex UART Started");
  TXPacket0x42.s.start = 0xFC;
  TXPacket0x42.s.source_id = 0x42;
  TXPacket0x42.s.target_id = 0x41;
  TXPacket0x42.s.byte7 = 0x01;
  TXPacket0x42.s.end = 0xFB;

  TXPacket0x45.s.start = 0xFC;
  TXPacket0x45.s.source_id = 0x45;
  TXPacket0x45.s.target_id = 0x41;
  TXPacket0x45.s.io_stat = 0x43;
  TXPacket0x45.s.batt_lvl = 0x64;
  TXPacket0x45.s.byte5 = 0x64;
  TXPacket0x45.s.byte6 = 0x00;
  TXPacket0x45.s.end = 0xFB;
}
void CommInterface::begin() {
  xTaskCreate(
    receive_thread,
    "RECEIVE_THREAD",
    10000,
    this,
    1,
    NULL);
}
uint16_t CommInterface::calculateChecksum(uint8_t* buffer) {

  uint16_t checksum = 0;
  if (buffer[1] == 0x41) {
    for (int i = 1; i < 11; i++) {
      checksum += buffer[i];
    }
  } else if (buffer[1] == 0x42) {
    for (int i = 1; i < 8; i++) {
      checksum += buffer[i];
    }
  } else if (buffer[1] == 0x45) {
    for (int i = 1; i < 7; i++) {
      checksum += buffer[i];
    }
  }
  return checksum;
}
void CommInterface::transmit(uint8_t* buffer, size_t size) {
  Serial2.end(true);
  Serial2.begin(9600, SERIAL_8N1, TXD2, RXD2, true);
  // Serial.print("TX:");
  for (int i = 0; i < size; i++) {
    // Serial.print(" ");
    // Serial.print(buffer[i], HEX);
    Serial2.write(buffer[i]);
  }
  // Serial.println();
  Serial2.flush();
  Serial2.end(true);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2, true);
}
void CommInterface::send_response(uint8_t response_type) {
  if (response_type == 0x42) {
    TXPacket0x42.s.checksum = calculateChecksum(TXPacket0x42.b);
    transmit(TXPacket0x42.b, sizeof(TXPacket0x42));
  } else if (response_type == 0x45) {
    TXPacket0x45.s.checksum = calculateChecksum(TXPacket0x45.b);
    transmit(TXPacket0x45.b, sizeof(TXPacket0x45));
  }
}
void CommInterface::receive_thread(void* parameter) {
  CommInterface* taskInstance = (CommInterface*)parameter;
  while (true) {
    if (Serial2.available() > 13) {
      uint8_t x = Serial2.read();
      if (x == 0xFB) {
        taskInstance->RXPacket0x41.b[0] = x;
        x = Serial2.read();
        if (x == 0x41) {
          taskInstance->RXPacket0x41.b[1] = x;
          x = Serial2.read();
          if ((x == 0x42) | (x == 0x43) | (x == 0x44) | (x == 0x45)) {
            taskInstance->RXPacket0x41.b[2] = x;
            for (int i = 3; i < 14; i++) {
              taskInstance->RXPacket0x41.b[i] = Serial2.read();
            }
            if (taskInstance->RXPacket0x41.s.end == 0xFC) {
              if (taskInstance->calculateChecksum(taskInstance->RXPacket0x41.b) == taskInstance->RXPacket0x41.s.checksum) {

                // Serial.print("RX:");
                // for (int i = 0; i < 14; i++) {
                //   Serial.print(" ");
                //   Serial.print(taskInstance->RXPacket0x41.b[i], HEX);
                // }
                // Serial.println();
                taskInstance->send_response(taskInstance->RXPacket0x41.s.target_id);
              } else {
                Serial.println("RX: Checksum Error");
              }
            } else {
              Serial.print("RX: Wrong end!:");
              Serial.println(x, HEX);
            }
          } else {
            Serial.print("RX: Wrong target id!:");
            Serial.println(x, HEX);
          }
        } else {
          Serial.print("RX: Wrong Source id!:");
          Serial.println(x, HEX);
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}