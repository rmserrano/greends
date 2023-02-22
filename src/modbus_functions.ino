/*
  modbus_functions.ino - FreeDs Modbus functions
  Derivador de excedentes para ESP32 DEV Kit // Wifi Kit 32
  
  Copyright (C) 2020 Theo arendst (https://github.com/arendst/Tasmota)
  Copyright (C) 2020 Pablo Zerón (https://github.com/pablozg/freeds)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

uint8_t mb_address;
uint8_t mb_len;

uint16_t modbusCalculateCRC(uint8_t *frame, uint8_t num)
{
  uint16_t crc = 0xFFFF;

  for (uint8_t i = 0; i < num; i++) {
    crc ^= frame[i];
    for (uint8_t j = 8; j; j--) {
      if ((crc & 0x0001) != 0) {        // If the LSB is set
        crc >>= 1;                      // Shift right and XOR 0xA001
        crc ^= 0xA001;
      } else {                          // Else LSB is not set
        crc >>= 1;                      // Just shift right
      }
    }
  }
  return crc;
}

void modbusSend(uint8_t device_address, uint8_t function_code, uint16_t start_address, uint16_t register_count)
{
  /*
  uint8_t frame[8];

  mb_address = device_address;  // Save address for receipt check

  frame[0] = mb_address;        // 0xFE default device address or dedicated like 0x01
  frame[1] = function_code;
  frame[2] = (uint8_t)(start_address >> 8);   // MSB
  frame[3] = (uint8_t)(start_address);        // LSB
  frame[4] = (uint8_t)(register_count >> 8);  // MSB
  frame[5] = (uint8_t)(register_count);       // LSB
  uint16_t crc = modbusCalculateCRC(frame, 6);
  frame[6] = (uint8_t)(crc);
  frame[7] = (uint8_t)(crc >> 8);

  SerieMeter.flush();
  SerieMeter.write(frame, sizeof(frame));
  */
}

void modbusSend_Response(uint8_t device_address, uint8_t *response, uint8_t len)
{
  /*
  mb_address = device_address;  // Save address for receipt check
  SerieMeter.flush();
  SerieMeter.write(response, len);
  */
}

bool modbusReceiveReady()
{
  /*
  return (SerieMeter.available() > 4);
  */
 return true;
}

uint8_t modbusReceiveBuffer(uint8_t *buffer, uint8_t register_count)
{
  /*
  mb_len = 0;
  uint32_t last = millis();
  while ((SerieMeter.available() > 0) && (mb_len < (register_count *2) + 5) && (millis() - last < 10)) {
    uint8_t data = (uint8_t)SerieMeter.read();
    if (!mb_len) {               // Skip leading data as provided by hardware serial
      if (mb_address == data) {
        buffer[mb_len++] = data;
      }
    } else {
      buffer[mb_len++] = data;
      if (3 == mb_len) {
        if (buffer[1] & 0x80) {  // 01 84 02 f2 f1
          return buffer[2];      // 1 = Illegal Function,
                                 // 2 = Illegal Data Address,
                                 // 3 = Illegal Data Value,
                                 // 4 = Slave Error
                                 // 5 = Acknowledge but not finished (no error)
                                 // 6 = Slave Busy
                                 // 8 = Memory Parity error
                                 // 10 = Gateway Path Unavailable
                                 // 11 = Gateway Target device failed to respond
        }
      }
    }
    last = millis();
  }

  if (mb_len < 7) { return 7; }  // 7 = Not enough data

  uint16_t crc = (buffer[mb_len -1] << 8) | buffer[mb_len -2];
  if (modbusCalculateCRC(buffer, mb_len -2) != crc) {
    return 9;                    // 9 = crc error
  }
*/
  return 0;                      // 0 = No error
}

uint8_t modbusReceive16BitRegister(uint16_t *value)
{
  //  0  1  2  3  4  5  6
  // 01 04 02 43 21 HH LL
  // Id Cc Sz Regis Crc--

  uint8_t buffer[7];
/*
  uint8_t error = modbusReceiveBuffer(buffer, 1);  // 1 x 16bit register
  if (!error) {
    *value = (buffer[3] << 8) | buffer[4];
  }

  return error;
  */
 return 0;
}

uint8_t modbusReceive32BitRegister(float *value)
{
  //  0  1  2  3  4  5  6  7  8
  // 01 04 04 87 65 43 21 HH LL
  // Id Cc Sz Register--- Crc--
/*
  uint8_t buffer[9];

  *value = NAN;

  uint8_t error = modbusReceiveBuffer(buffer, 2);  // 1 x 32bit register
  if (!error) {
    ((uint8_t*)value)[3] = buffer[3];
    ((uint8_t*)value)[2] = buffer[4];
    ((uint8_t*)value)[1] = buffer[5];
    ((uint8_t*)value)[0] = buffer[6];
  }

  return error;
  */
 return 0;
}