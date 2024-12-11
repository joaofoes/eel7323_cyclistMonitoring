#include <TinyGPS++.h>
#include <HardwareSerial.h>

// TinyGPS++ instance
TinyGPSPlus gps;

// UART configuration for GPS
HardwareSerial SerialGPS(1); // Use UART1 for GPS

#define RXD2 16 // RX pin for GPS
#define TXD2 17 // TX pin for GPS

void setup() {
  Serial.begin(115200); // Debug output
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2); // GPS communication
  Serial.println("Starting GPS...");
}

void loop() {
  // Read data from GPS
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    gps.encode(c); // Pass data to TinyGPS++

    // Check and print time if updated
    if (gps.time.isUpdated()) {
      Serial.print("Time (UTC): ");
      Serial.printf("%02d:%02d:%02d\n", gps.time.hour(), gps.time.minute(), gps.time.second());
    }

    // Check and print speed if updated
    if (gps.speed.isUpdated()) {
      Serial.print("Speed (km/h): ");
      Serial.println(gps.speed.kmph());
    }
  }
}
