/*
24C32.cpp - Library for EEPROM 24C32
Franz Stoiber 2020
*/
  
#include "24C32.h"
#include <Wire.h>

bool EE::begin(uint8_t I2CAdr) {
  //I2CAdr ... one of the posible device addresses
  //result is true when device present else false
  DeviceAdr = I2CAdr;
  Wire.beginTransmission(DeviceAdr);
  if (Wire.endTransmission() != 0) {
    Serial.println("24C32: device not found");
    return(false);
  }
  return(true);
}

bool EE::eraseData(uint8_t Val, bool Check) {
  //Val ... erase to this value
  //Check ... check the result
  //result is true when check ok else false
  uint16_t i, j;
  uint16_t Adr;
  bool Ok;
  for (i = 0; i < EEPROM24C32__PAGES; i++) {
    Adr = i * EEPROM24C32__PAGE_SIZE;
    Wire.beginTransmission(DeviceAdr);
    Wire.write(Adr >> 8);
    Wire.write(Adr & 0xFF); 
    for (j = 0; j < I2C_WRITE_BUFFER_SIZE; j++) Wire.write(Val);
    Wire.endTransmission();
    waitReady();
    Wire.beginTransmission(DeviceAdr);
    Adr += I2C_WRITE_BUFFER_SIZE;
    Wire.write(Adr >> 8);
    Wire.write(Adr & 0xFF); 
    for (j = I2C_WRITE_BUFFER_SIZE; j < EEPROM24C32__PAGE_SIZE; j++) Wire.write(Val);
    Wire.endTransmission();
    waitReady();
  }
  Ok = true;
  if (Check) {
    uint8_t Result[EEPROM24C32__SIZE];
    readBytes(0, EEPROM24C32__SIZE, Result);
    for (i = 0; i < EEPROM24C32__SIZE; i++) {
      if (Result[i] != Val) {
        Serial.print("EE erase data on address ");
        Serial.print(i);
        Serial.println(" failed");
        Ok = false;
      }
    }
  }
  return(Ok); 
}

bool EE::writeByte(uint16_t Adr, uint8_t Data, bool Check, bool Update) {
  //Adr    ... address in EEPROM
  //Data   ... data byte to write
  //Check  ... check the result
  //Update ... write only if data differs from EEPROM value
  //result is true when check ok else false
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE write byte address invalid");
  if (Update && readByte(Adr) == Data) return(true); 
  Wire.beginTransmission(DeviceAdr);
  Wire.write(Adr >> 8);
  Wire.write(Adr & 0xFF);
  Wire.write(Data);
  Wire.endTransmission();
  waitReady();
  if (Check && readByte(Adr) != Data) {
    Serial.println("EE write byte failed");
    return(false);    
  }
  return(true);
}

bool EE::writeFloat(uint16_t Adr, float Val, bool Check, bool Update) {
  //Adr    ... address in EEPROM
  //Val    ... value to write
  //Check  ... check the result
  //Update ... write only if data differs from EEPROM value
  //result is true when check ok else false
  const uint8_t Size = sizeof(float);
  union {
    float Float;
    uint8_t Bytes[Size];
  } Value;
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE write float address invalid");
  if (Update && readFloat(Adr) == Val) return(true);   
  Value.Float = Val;
  writeBytes(Adr, Size, Value.Bytes, false);
  if (Check && readFloat(Adr) != Val) {
    Serial.println("EE write float failed");
    return(false);    
  } 
  return(true);
}

bool EE::writeDouble(uint16_t Adr, double Val, bool Check, bool Update) {
  //Adr    ... address in EEPROM
  //Val    ... value to write
  //Check  ... check the result
  //Update ... write only if data differs from EEPROM value
  //result is true when check ok else false
  const uint8_t Size = sizeof(double);
  union {
    double Double;
    uint8_t Bytes[Size];
  } Value;
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE write double address invalid");
  if (Update && readDouble(Adr) == Val) return(true); 
  Value.Double = Val;
  writeBytes(Adr, Size, Value.Bytes, false);
  if (Check && readDouble(Adr) != Val) {
    Serial.println("EE write double failed");
    return(false);    
  } 
  return(true);
}

bool EE::writeBytes(uint16_t Adr, int16_t Len, uint8_t *Data, bool Check) {
  //Adr    ... address in EEPROM
  //Len    ... nr of bytes to write
  //Data   ... array of bytes to write
  //Check  ... check the result
  //result is true when check ok else false
  uint8_t PageNr;
  uint8_t StartByteNr;
  int8_t Length0;
  int8_t Length1;
  uint8_t i;
  uint16_t Pos;
  bool Ok;
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE writwe bytes address invalid");
  if ((Adr + Len) > EEPROM24C32__SIZE) Serial.println("EE write bytes length invalid");
  Pos = 0;
  Ok = true;
  while (Len > 0) {
    PageNr = Adr / EEPROM24C32__PAGE_SIZE;
    StartByteNr = Adr - EEPROM24C32__PAGE_SIZE * PageNr;
    Length0 = EEPROM24C32__PAGE_SIZE - StartByteNr;
    if (Length0 > Len) Length0 = Len;
    Length1 = 0;
    if (Length0 > I2C_WRITE_BUFFER_SIZE) {
      Length1 = Length0 - I2C_WRITE_BUFFER_SIZE;
      Length0 = I2C_WRITE_BUFFER_SIZE;
    }
    if (!writePage(PageNr, StartByteNr, Length0, Data, Pos, Check)) Ok = false;
    Adr += Length0;
    Len -= Length0;
    StartByteNr += I2C_WRITE_BUFFER_SIZE;
    if (Length1 > 0) {
      if (!writePage(PageNr, StartByteNr, Length1, Data, Pos, Check)) Ok = false;
      Adr += Length1;
      Len -= Length1;
    }
  }
  return(Ok);
}

uint8_t EE::readByte(uint16_t Adr) {
  //Adr    ... address in EEPROM
  //result is read byte
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE read byte address invalid");
  uint32_t TimeoutMillis;
  Wire.beginTransmission(DeviceAdr);
  Wire.write(Adr >> 8);
  Wire.write(Adr & 0xFF);
  Wire.endTransmission();
  TimeoutMillis = millis() + 5;
  Wire.requestFrom(DeviceAdr, 1);
  while (Wire.available() < 1) {
    if ((int32_t)(millis() - TimeoutMillis) > 0) {
      Serial.println("EE read byte timeout");
      break;
    }
  }
  return(Wire.read());
}

float EE::readFloat(uint16_t Adr) {
  //Adr    ... address in EEPROM
  //result is read value
  const uint8_t Size = sizeof(float);
  union {
    float Float;
    uint8_t Bytes[Size];
  } Value;
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE read float address invalid");
  readBytes(Adr, Size, Value.Bytes);
  return(Value.Float); 
}

double EE::readDouble(uint16_t Adr) {
  //Adr    ... address in EEPROM
  //result is read value
  const uint8_t Size = sizeof(double);
  union {
    double Double;
    uint8_t Bytes[Size];
  } Value;
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE read double address invalid");
  readBytes(Adr, Size, Value.Bytes);
  return(Value.Double); 
}

void EE::readBytes(uint16_t Adr, int16_t Len, uint8_t *Data) {
  //Adr    ... address in EEPROM
  //Len    ... nr of bytes to read
  //Data   ... array of bytes to read
  if (Adr >= EEPROM24C32__SIZE) Serial.println("EE read bytes address invalid");
  if ((Adr + Len) > EEPROM24C32__SIZE) Serial.println("EE read bytes length invalid");
  uint16_t i;
  uint16_t Pos;
  int16_t Length;
  uint32_t TimeoutMillis;
  Wire.beginTransmission(DeviceAdr);
  Wire.write(Adr >> 8);
  Wire.write(Adr & 0xFF);
  Wire.endTransmission();  
  Pos = 0;   
  while (Len > 0) {   
    Length = Len;
    if (Length > I2C_READ_BUFFER_SIZE) Length = I2C_READ_BUFFER_SIZE;                                               
    TimeoutMillis = millis() + 10;
    Wire.requestFrom(DeviceAdr, Length);
    while (Wire.available() < Length) {
      if ((int32_t)(millis() - TimeoutMillis) > 0) {
        //Serial.println("EE read bytes timeout");
        break;
      }
    }
    for (i = 0; i < Length; i++) {
      Data[Pos] = Wire.read();
      Pos += 1;
    }
    Len -= Length;                
  }
} 

bool EE::writePage (uint8_t PageNr, uint8_t StartByteNr, int8_t Len, uint8_t *Data, uint16_t &Pos, bool Check) {
  uint16_t Adr;
  uint8_t i;
  uint16_t StartPos;
  bool Ok;
  if (PageNr >= EEPROM24C32__PAGES) Serial.println("EE write page nr invalid");
  if (StartByteNr >= EEPROM24C32__PAGE_SIZE) Serial.println("EE write page byte nr invalid");
  if ((StartByteNr + Len) > EEPROM24C32__PAGE_SIZE) Serial.println("EE write page length invalid");
  StartPos = Pos;
  Adr = (PageNr << 5) | StartByteNr; 
  Wire.beginTransmission(DeviceAdr);
  Wire.write(Adr >> 8);
  Wire.write(Adr & 0xFF);
  for (i = 0; i < Len; i++) {
    Wire.write(Data[Pos]);
    Pos += 1;
  }
  Wire.endTransmission();
  waitReady();
  Ok = true;
  if (Check) {   
    uint8_t Result[I2C_WRITE_BUFFER_SIZE];
    readBytes(Adr, Len, Result);
    for (i = 0; i < Len; i++) {
      if (Result[i] != Data[StartPos + i]) Ok = false;
    }
    if (!Ok) {
      Serial.print("EE write page ");
      Serial.print(PageNr);
      Serial.println(" failed");
    }
  }
  return(Ok); 
}

void EE::waitReady() {
  uint8_t Response;
  uint32_t TimeoutMillis;
  TimeoutMillis = millis() + 10;
  while (true) {
    Wire.beginTransmission(DeviceAdr);
    Response = Wire.endTransmission();
    if (Response == 0) break;
    if ((int32_t)(millis() - TimeoutMillis) > 0) {
      Serial.println("EE wait ready timeout");
      break;
    }
  }
}
