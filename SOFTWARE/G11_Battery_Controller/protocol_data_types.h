#pragma pack(push, 1)
struct RXPacket0x41_t {
  uint8_t start;         //0=0xFB
  uint8_t source_id;     //1=0x41
  uint8_t target_id;     //2=0x42,0x43,0x44,0x45
  uint32_t motor_speed;  //3,4,5,6 motor speed in RPM
  uint8_t byte7;         //7=0x00
  uint8_t byte8;         //8=0x00
  uint8_t byte9;         //9=0x09
  uint8_t byte10;        //10=0x00
  uint16_t checksum;     //11,12
  uint8_t end;           //13=0xFC
};
struct TXPacket0x42_t {
  uint8_t start;               //0=0xFC
  uint8_t source_id;           //1=0x42
  uint8_t target_id;           //2=0x41
  uint16_t power_consumption;  //3,4 power consumption in WATTS
  uint16_t current;            //5,6 current in milliAMPS
  uint8_t byte7;               //7=0x01
  uint16_t checksum;           //8,9
  uint8_t end;                 //10=0xFB
};
struct TXPacket0x45_t {
  uint8_t start;      //0=0xFC
  uint8_t source_id;  //1=0x45
  uint8_t target_id;  //2=0x41
  uint8_t io_stat;    //3=charging(0x43),not charging(0x44)
  uint8_t batt_lvl;   //4=level percentage
  uint8_t byte5;      //5=0x64
  uint8_t byte6;      //6=0x00
  uint16_t checksum;  //7,8
  uint8_t end;        //9=0xFB
};
#pragma pack(pop)
union RXPacket0x41_u {
  struct RXPacket0x41_t s;
  uint8_t b[14];
};
union TXPacket0x42_u {
  struct TXPacket0x42_t s;
  uint8_t b[11];
};
union TXPacket0x45_u {
  struct TXPacket0x45_t s;
  uint8_t b[10];
};