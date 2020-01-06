/*
  24C32 example -  by Franz Stoiber

  Hardware ----------------------------------------------------
  controller: ESP32 Dev Module
  EEPROM 24C32
  -------------------------------------------------------------

  Pin usage ---------------------------------------------------
  Pin  3V3                          ... 3V3
  Pin  GND                          ... 0V
  Pin  D15/ADC2_3/TOUCH3/SPI-SS     ...
  Pin  D02/ADC2_2/TOUCH02           ...
  Pin  D04/ADC2_0/TOUCH00           ...
  Pin  D16/RX2                      ...
  Pin  D17/TX2                      ...
  Pin  D05/VSPI-SS                  ...
  Pin  D18/VSPI-SCK                 ...
  Pin  D19/VSPI-MISO                ...
  Pin  D21/I2C-SDA                  ...  SDA
  Pin  D03/RX0                      ...
  Pin  D01/TX0                      ...
  Pin  D22/I2C-SCL                  ...  SCL
  Pin  D23/VSPI-MOSI                ...
  Pin  EN/RESET                     ...
  Pin  VP only in                   ...
  Pin  VN only in                   ...
  Pin  D34/ADC1_6 only in           ...
  Pin  D35/ADC1_7 only in           ...
  Pin  D32/ADC1_4/TOUCH09           ...
  Pin  D33/ADC1_5/TOUCH08           ...
  Pin  D25/ADC2_8/DAC1              ...
  Pin  D26/ADC2_9/DAC2              ...
  Pin  D27/ADC2_7/TOUCH07           ...
  Pin  D14/ADC2_6/TOUCH06/SPI-CLK   ...
  Pin  D12/ADC2_5/TOUCH05/SPI-MISO  ...
  Pin  D13/ADC2_4/TOUCH04/SPI-MOSI  ...
  Pin  GND                          ... 0V
  Pin  VIN                          ...
  Pin  IO0 (only on breakout)       ...
  -------------------------------------------------------------
*/

//includes
#include "24C32.h"
#include <Wire.h>

//declarations for prg
String PrgName = "24C32 example";
String PrgVersion = "1.0";
uint8_t a[500];
uint8_t b[EEPROM24C32__SIZE];

//instance 24C32
EE myEEPROM;

void setup() {
  uint16_t i;
  bool Ok;

  Serial.begin(115200);
  Serial.print(PrgName);
  Serial.print("  V");
  Serial.println(PrgVersion);

  //init library Wire
  Wire.begin();
  Wire.setClock(400000);

  //init class EEPROM24C3231
  myEEPROM.begin(EEPROM24C32_DEFAULT_ADDRESS);

  Serial.println("--- erase all data in EEPROM");
  Ok = myEEPROM.eraseData(0xFF, true);
  if (Ok) Serial.println("erase data ok");
  else Serial.println("erase data failed");

  Serial.println("--- do some writes and reads");
  myEEPROM.writeByte(100, 0x77, true, false);
  Serial.println(myEEPROM.readByte(100), HEX);
  myEEPROM.writeFloat(101, 547.451, true, false);
  Serial.println(myEEPROM.readFloat(101), 3);
  myEEPROM.writeDouble(105, 3457.485967, true, false);
  Serial.println(myEEPROM.readDouble(105), 6);

  Serial.println("--- write 500 bytes");
  for (i = 0; i < 500; i++) a[i] = i % 256;
  Ok = myEEPROM.writeBytes(1000, 500, a, true);
  if (Ok) Serial.println("write bytes ok");
  else Serial.println("write bytes failed");

  Serial.println("--- read 500 bytes");
  myEEPROM.readBytes(1000, 500, b);
  for (i = 0; i < 500; i += 8) {
    Serial.print(i);
    Serial.print(":  ");
    Serial.print(b[i]);
    Serial.print("  ");
    Serial.print(b[i + 1]);
    Serial.print("  ");
    Serial.print(b[i + 2]);
    Serial.print("  ");
    Serial.print(b[i + 3]);
    Serial.print("  ");
    Serial.print(b[i + 4]);
    Serial.print("  ");
    Serial.print(b[i + 5]);
    Serial.print("  ");
    Serial.print(b[i + 6]);
    Serial.print("  ");
    Serial.println(b[i + 7]);
  }

  Serial.println("--- read all bytes");
  myEEPROM.readBytes(0, EEPROM24C32__SIZE, b);
  for (i = 0; i < EEPROM24C32__SIZE; i += 8) {
    Serial.print(i);
    Serial.print(":  ");
    Serial.print(b[i]);
    Serial.print("  ");
    Serial.print(b[i + 1]);
    Serial.print("  ");
    Serial.print(b[i + 2]);
    Serial.print("  ");
    Serial.print(b[i + 3]);
    Serial.print("  ");
    Serial.print(b[i + 4]);
    Serial.print("  ");
    Serial.print(b[i + 5]);
    Serial.print("  ");
    Serial.print(b[i + 6]);
    Serial.print("  ");
    Serial.println(b[i + 7]);
  }

  Serial.println("--- end ");
}

void loop() {
  Serial.println("loop");
  delay(30000);
}
