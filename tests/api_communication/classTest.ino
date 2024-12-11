#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Google_Sheet_Client.h>

// WiFi Credentials
#define WIFI_SSID 
#define WIFI_PASSWORD

// Google Cloud Project Details
#define PROJECT_ID 

// Service Account Credentials
#define CLIENT_EMAIL 

// Spreadsheet Details
#define SPREADSHEET_ID 
#define SHEET_NAME 

// Structure TripData for trip details
struct TripData {
    float velocidadeMedia = 0.0f;
    float distanciaTotal = 0.0f;
    String tagRFID;
    String startDate;
    String startTime;
    String endDate;
    String endTime;
};

// GoogleSheetsSender Class
class GoogleSheetsSender {
public:
    GoogleSheetsSender() {
        GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
    }

    void sendTripData(const TripData &data) {
        int verif = -1;
        while (verif != 0) {
          if (GSheet.ready()){
            verif = 0;
            FirebaseJson response;
            FirebaseJson json;
            json.add("majorDimension", "COLUMNS");
            json.set("values/[0]/[0]", data.velocidadeMedia);
            json.set("values/[1]/[0]", data.distanciaTotal);
            json.set("values/[2]/[0]", data.tagRFID);
            json.set("values/[3]/[0]", data.startDate);
            json.set("values/[4]/[0]", data.startTime);
            json.set("values/[5]/[0]", data.endDate);
            json.set("values/[6]/[0]", data.endTime);

            if (GSheet.values.append(&response, SPREADSHEET_ID, SHEET_NAME, &json)) {
                Serial.println("Trip data sent successfully.");
                response.toString(Serial, true);
            } else {
                Serial.println("Failed to send trip data.");
            }
          } else {
            Serial.println("Google Sheet client is not ready.");
          }
        } 
    }

private:
    ESP_Google_Sheet_Client googleSheetClient;
};

// Instance of GoogleSheetsSender
GoogleSheetsSender sheetSender;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");
  
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  unsigned long startAttemptTime = millis();
  // Wait for connection with timeout (e.g., 10 seconds)
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("Failed to connect to Wi-Fi.");
    // Handle reconnection or enter deep sleep
  }

    // Example TripData
    TripData trip;
    trip.velocidadeMedia = 15.2f;
    trip.distanciaTotal = 23.7f;
    trip.tagRFID = "RFID12345";
    trip.startDate = "2024-11-13";
    trip.startTime = "10:00:00";
    trip.endDate = "2024-11-13";
    trip.endTime = "11:30:00";
    
    // Send example data
    sheetSender.sendTripData(trip);
}

void loop() {
    // Future trip data sending can be implemented here
}
