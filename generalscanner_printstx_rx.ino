// General-purpose UART Scanner

// User-defined baud rate
#define DEBUG_BAUD 9600   // Baud rate for Serial Monitor
#define SCAN_BAUD 4800   // Change this to the baud rate you want to scan

void setup() {
  Serial.begin(DEBUG_BAUD);            // Start Serial Monitor
  Serial1.begin(SCAN_BAUD);            // UART line to scan
  Serial.println("UART Scanner Initialized");
  Serial.print("Scanning at baud: ");
  Serial.println(SCAN_BAUD);
}

void loop() {
  // Read from UART line and print to Serial Monitor
  if (Serial1.available()) {
    byte incoming = Serial1.read();
    Serial.print("RX: 0x");
    if (incoming < 16) Serial.print("0");
    Serial.print(incoming, HEX);
    Serial.print(" (");
    Serial.print((char)incoming);
    Serial.println(")");
  }

  // Optionally, echo data sent from Serial Monitor to UART line
  if (Serial.available()) {
    byte outgoing = Serial.read();
    Serial1.write(outgoing);
    Serial.print("TX: 0x");
    if (outgoing < 16) Serial.print("0");
    Serial.print(outgoing, HEX);
    Serial.print(" (");
    Serial.print((char)outgoing);
    Serial.println(")");
  }
}
