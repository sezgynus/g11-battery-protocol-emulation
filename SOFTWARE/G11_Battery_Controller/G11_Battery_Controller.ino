#define ESP32_RTOS
#include "OTA.h"
#include "comm_interface.h"
void setup() {
  setupOTA(mySSID, mySSID, myPASSWORD);
  CI.init();
  CI.begin();
  pinMode(26, INPUT_PULLUP);
  pinMode(32, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(27, OUTPUT);
  digitalWrite(32, HIGH);
  digitalWrite(25, LOW);
  digitalWrite(27, LOW);
  analogReadResolution(12);
}

void loop() {
  double v_batt = analogReadMilliVolts(33) * 8.547008547008547;  // Vout/Vin = 0.117 voltage divider. 91Kohms/12Kohms
  uint8_t batt_percentage = map(v_batt, 18000, 24900, 0, 100);   // %100 = 24.9V %0=18V
  if (batt_percentage > 100) batt_percentage = 100;
  if (batt_percentage < 0) batt_percentage = 0;

  CI.TXPacket0x45.s.batt_lvl = batt_percentage;
  bool charger_status = digitalRead(26);
  bitWrite(CI.TXPacket0x45.s.io_stat, 0, charger_status);
  bitWrite(CI.TXPacket0x45.s.io_stat, 1, charger_status);
  bitWrite(CI.TXPacket0x45.s.io_stat, 2, !charger_status);
  delay(250);

  Serial.print("Charger Status: ");
  Serial.print(charger_status);
  Serial.print(" - Battery Status: ");
  Serial.println(CI.TXPacket0x45.s.batt_lvl);
}
void Received_event() {
}
