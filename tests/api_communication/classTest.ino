#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Google_Sheet_Client.h>

// WiFi Credentials
#define WIFI_SSID "NET_2G7B25AD"
#define WIFI_PASSWORD "4F7B25AD"

// Google Cloud Project Details
#define PROJECT_ID "monitor-cilcista"

// Service Account Credentials
#define CLIENT_EMAIL "monitorciclista@monitor-cilcista.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC99JHBNy/JhkMT\nRYmEC5RRVCmz3FWhDHFOnGErGY+g0n8mEfTpPg92x6Fn4T4m7PjGEqiOYnhIUdyw\neEcZp6Iu7kCV9AoLK+KynBrri9ex5FguINeIXAzunoZHnXDgqEzVwNC1lyQyTdb7\nv1IB7gI+nHWIIHsEntPBy+RZO778UQutAmZiUp0Ya8dKEU9s9Ue0RSPWpvwQpo8T\nmSM0QQe0rzOtKcvzX2NPIw6Al4JXaOJoM3nZ1gKQgO7qL9cTTa6AITGUiwUkrTT/\nUz8lybnXiQL0THCsY0td1jxoQdTyQ7LaTerZKsY3dcAdNyHGVfL2wAuS74vnLBWR\n+7Z1YFiXAgMBAAECggEAFy1qxW0S5x97OrQ+4b459Cx1RC1rw1FN1/4ruwMM694T\nRpVlUTpRz7fvAUpR95Tti9mF+I2aSJS4fU3HuyEhoGFQvxs+SHur1whGHpx7Doki\nm0pFki1NJKZyEvXqancOnPxAWw/cikjGJQSEoJimbHZRvvDbocYxZ1GWBK9OD+Ir\n3mPvjLcjW10cRhhBfRgU0kyGlPzMaxSfsodBYFxlSyREiFDfYA+la6kv1llcjf9j\n9o4Mxy+JdiFRpZ0ByLzC3cdUkk01qywZvgC+n31ELJgVUm6I/V8LJMeaBJPX3URJ\nQtUMJE4U5BwxKuwzUJPMd6nH6BeNgMSgmq0wjM12aQKBgQDd2Rn23GeUbVa+9V27\nvnMH2MymD6zpr+z2g72hSvbR4BOn7IP7tPiojRKuC5pQNEQ/KRWIX1clKu6b6d5G\nYcVzg8p6Qk1ZsT9WUuYVVHR9cecDfxIUN+s595RuyAb4y8Sab/ICMxTMF5MG59Bo\nKaSgFm60caeY1MZ7NhmcqzxUmQKBgQDbMph7ih824WUcdgsJVoQb5LYrjIHQbs9O\n/SefslRB+oa3GMkJ60rQW5FUSl896CzRimGogtM7qBqNC3h3K6XriqqteQRRTaDG\nWwf+vEFZmSauwAiEkk4UxD4INK7tuPjDgrIfktKAUjWAt1mwECGs6rbe24IA/Njn\nzpyrzkUkrwKBgC9eEG4mBzOo2NGzAA17D4KIZsG6LRhESNslq9KJeAO9zVSYamHO\n9Ry/6lIhBhTD5CJ8Oi3A9ZFpBkNh/uSWMZuFczEBgRR6hOSAlMHPI4Q5UrLracfJ\nXL8mvpmcuehbUaIL7udLUtB05B6gVl43LgBEaeS7M6atH6flGDjcUuhZAoGAeHvk\nEqvGIo1b3XwI7vYOIaLieyd9R0tRShKekAoehDGnkMbf4kLh2YsrY9CJb2bKE9dj\nfJOEdhsdlP+EFMT6K3EKBdYqCGKKB92LlHp31T74Yz+MrwoYPxPrIOmMaYCtxBF3\nxSEX8Y6+d+TQOJV1XS2anP69A7v3odsQhOMZYGMCgYEAqUxIgZ07fpWgE4Z9yy9r\n8GK5r5g8FQCcv4Ay34BhauE+4PDqSixLjiA1YoZ72CkuHSKlaev+FnA4TI0PbiXT\ntNFuhoVxwp13MIMfDL6z4BXSVViiOPQJYhmkHQAlndWJEEwDEixjhWJV+DBM4WJB\nO86pB3n2POvBckFOBf5cSWA=\n-----END PRIVATE KEY-----\n";

// Spreadsheet Details
#define SPREADSHEET_ID "1D0T8XxEPtv21ZRA23hG3YEpjcttYyo2njtz9Ztr2vnE"
#define SHEET_NAME "Sheet1"

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
