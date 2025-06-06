#include <Arduino.h>

#define SENSOR_ID 0x01
#define UART Serial1

// Manual CRC Calculation Function (Modbus CRC16)
uint16_t ModbusCRC16(uint8_t *buffer, uint8_t len) {
  uint16_t crc = 0xFFFF;
  for (uint8_t pos = 0; pos < len; pos++) {
    crc ^= buffer[pos];
    for (uint8_t i = 0; i < 8; i++) {
      if (crc & 0x0001)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }
  return crc;
}

// Send Modbus command manually
void sendModbusCommand(uint8_t *cmd, uint8_t len) {
  for (uint8_t i = 0; i < len; i++) {
    UART.write(cmd[i]);
  }
}

// Receive response manually
bool receiveModbusResponse(uint8_t *response, uint8_t expectedLen, uint16_t timeout_ms) {
  uint32_t startTime = millis();
  uint8_t index = 0;
  while (millis() - startTime < timeout_ms && index < expectedLen) {
    if (UART.available()) {
      response[index++] = UART.read();
    }
  }
  return index == expectedLen;
}

void setup() {
  Serial.begin(9600);
  UART.begin(4800);
  Serial.println("Manual Modbus RTU - IRNET-PRO");
}

void loop() {
  // Construct request to read concentration (input registers at 0x0006)
  uint8_t request[] = {SENSOR_ID, 0x04, 0x00, 0x06, 0x00, 0x02};  // no CRC yet
  uint16_t crc = ModbusCRC16(request, 6);
  
  // Append CRC to request
  uint8_t modbus_request[8] = {
    request[0], request[1], request[2], request[3],
    request[4], request[5],
    lowByte(crc), highByte(crc)
  };

  Serial.print("\nSending Modbus request: ");
  for (uint8_t i = 0; i < 8; i++) {
    if(modbus_request[i] < 0x10) Serial.print("0");
    Serial.print(modbus_request[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  // Flush serial buffer before sending
  while(UART.available()) UART.read();
  
  sendModbusCommand(modbus_request, 8);

  // Expected response: [Addr, Func, ByteCount(4), Data(4), CRC(2)] = 9 bytes
  uint8_t response[9];
  if (receiveModbusResponse(response, 9, 1000)) {
    Serial.print("Response received: ");
    for (uint8_t i = 0; i < 9; i++) {
      if(response[i] < 0x10) Serial.print("0");
      Serial.print(response[i], HEX);
      Serial.print(" ");
    }
    Serial.println();

    // Check CRC of response
    uint16_t respCRC = (response[8] << 8) | response[7];
    if (ModbusCRC16(response, 7) == respCRC) {
      // Response bytes: response[3] response[4] response[5] response[6]
      // Little-endian format from the sensor: (byte order: low to high)
      uint32_t concentrationRaw = ((uint32_t)response[6] << 24) |
                                  ((uint32_t)response[5] << 16) |
                                  ((uint32_t)response[4] << 8)  |
                                  ((uint32_t)response[3]);

      float concentration;
      memcpy(&concentration, &concentrationRaw, sizeof(concentration));
      Serial.print("Concentration: ");
      Serial.println(concentration, 3);
    } else {
      Serial.println("CRC error in response!");
    }
  } else {
    Serial.println("No response or incomplete response received.");
  }

  delay(5000);
}
