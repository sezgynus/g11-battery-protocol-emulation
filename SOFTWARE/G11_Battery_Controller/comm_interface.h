#include "Arduino.h"
#include "protocol_data_types.h"

#define RXD2 16
#define TXD2 17
void Received_event();

class CommInterface {
public:
  void init();
  void begin();
  RXPacket0x41_u RXPacket0x41;
  TXPacket0x45_u TXPacket0x45;
  TXPacket0x42_u TXPacket0x42;
private:
  static void receive_thread(void* parameter);
  uint16_t calculateChecksum(uint8_t* buffer);
  void transmit(uint8_t* buffer, size_t size);
  void send_response(uint8_t response_type);
};
extern CommInterface CI;