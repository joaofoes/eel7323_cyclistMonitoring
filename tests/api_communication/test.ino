#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Google_Sheet_Client.h>
// #include <FirebaseJson.h>  // Ensure this library is included

// WiFi Credentials
#define WIFI_SSID "NET_2G7B25AD"  // Your WiFi SSID
#define WIFI_PASSWORD "4F7B25AD"  // Your WiFi password

// Google Cloud Project Details
#define PROJECT_ID "monitor-cilcista"

// Service Account Credentials
#define CLIENT_EMAIL "monitorciclista@monitor-cilcista.iam.gserviceaccount.com"

// Service Account's Private Key (ensure proper formatting with \n)
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC99JHBNy/JhkMT\nRYmEC5RRVCmz3FWhDHFOnGErGY+g0n8mEfTpPg92x6Fn4T4m7PjGEqiOYnhIUdyw\neEcZp6Iu7kCV9AoLK+KynBrri9ex5FguINeIXAzunoZHnXDgqEzVwNC1lyQyTdb7\nv1IB7gI+nHWIIHsEntPBy+RZO778UQutAmZiUp0Ya8dKEU9s9Ue0RSPWpvwQpo8T\nmSM0QQe0rzOtKcvzX2NPIw6Al4JXaOJoM3nZ1gKQgO7qL9cTTa6AITGUiwUkrTT/\nUz8lybnXiQL0THCsY0td1jxoQdTyQ7LaTerZKsY3dcAdNyHGVfL2wAuS74vnLBWR\n+7Z1YFiXAgMBAAECggEAFy1qxW0S5x97OrQ+4b459Cx1RC1rw1FN1/4ruwMM694T\nRpVlUTpRz7fvAUpR95Tti9mF+I2aSJS4fU3HuyEhoGFQvxs+SHur1whGHpx7Doki\nm0pFki1NJKZyEvXqancOnPxAWw/cikjGJQSEoJimbHZRvvDbocYxZ1GWBK9OD+Ir\n3mPvjLcjW10cRhhBfRgU0kyGlPzMaxSfsodBYFxlSyREiFDfYA+la6kv1llcjf9j\n9o4Mxy+JdiFRpZ0ByLzC3cdUkk01qywZvgC+n31ELJgVUm6I/V8LJMeaBJPX3URJ\nQtUMJE4U5BwxKuwzUJPMd6nH6BeNgMSgmq0wjM12aQKBgQDd2Rn23GeUbVa+9V27\nvnMH2MymD6zpr+z2g72hSvbR4BOn7IP7tPiojRKuC5pQNEQ/KRWIX1clKu6b6d5G\nYcVzg8p6Qk1ZsT9WUuYVVHR9cecDfxIUN+s595RuyAb4y8Sab/ICMxTMF5MG59Bo\nKaSgFm60caeY1MZ7NhmcqzxUmQKBgQDbMph7ih824WUcdgsJVoQb5LYrjIHQbs9O\n/SefslRB+oa3GMkJ60rQW5FUSl896CzRimGogtM7qBqNC3h3K6XriqqteQRRTaDG\nWwf+vEFZmSauwAiEkk4UxD4INK7tuPjDgrIfktKAUjWAt1mwECGs6rbe24IA/Njn\nzpyrzkUkrwKBgC9eEG4mBzOo2NGzAA17D4KIZsG6LRhESNslq9KJeAO9zVSYamHO\n9Ry/6lIhBhTD5CJ8Oi3A9ZFpBkNh/uSWMZuFczEBgRR6hOSAlMHPI4Q5UrLracfJ\nXL8mvpmcuehbUaIL7udLUtB05B6gVl43LgBEaeS7M6atH6flGDjcUuhZAoGAeHvk\nEqvGIo1b3XwI7vYOIaLieyd9R0tRShKekAoehDGnkMbf4kLh2YsrY9CJb2bKE9dj\nfJOEdhsdlP+EFMT6K3EKBdYqCGKKB92LlHp31T74Yz+MrwoYPxPrIOmMaYCtxBF3\nxSEX8Y6+d+TQOJV1XS2anP69A7v3odsQhOMZYGMCgYEAqUxIgZ07fpWgE4Z9yy9r\n8GK5r5g8FQCcv4Ay34BhauE+4PDqSixLjiA1YoZ72CkuHSKlaev+FnA4TI0PbiXT\ntNFuhoVxwp13MIMfDL6z4BXSVViiOPQJYhmkHQAlndWJEEwDEixjhWJV+DBM4WJB\nO86pB3n2POvBckFOBf5cSWA=\n-----END PRIVATE KEY-----\n";


// Spreadsheet Details
#define SPREADSHEET_ID "1D0T8XxEPtv21ZRA23hG3YEpjcttYyo2njtz9Ztr2vnE"
#define SHEET_NAME "Sheet1"

bool taskComplete = false;

// Linked list node structure
struct Node {
  float velocidadeMedia;
  float distanciaTotal;
  int tempoTotal;    // Time in seconds
  char tagRFID[32];  // Using character array instead of String
  Node* next;
};

// Head of the linked list
Node* head = nullptr;

// Function to add a node to the linked list
void addNode(float vel, float dist, int tempo, const char* tag) {
  Node* newNode = new Node;
  newNode->velocidadeMedia = vel;
  newNode->distanciaTotal = dist;
  newNode->tempoTotal = tempo;
  strncpy(newNode->tagRFID, tag, sizeof(newNode->tagRFID) - 1);
  newNode->tagRFID[sizeof(newNode->tagRFID) - 1] = '\0';  // Ensure null-termination
  newNode->next = head;
  head = newNode;
}

// Function to clean up the linked list
void cleanUpList() {
  Node* current = head;
  while (current != nullptr) {
    Node* temp = current;
    current = current->next;
    delete temp;
  }
  head = nullptr;
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initializing...");

  // Initialize linked list with example data
  addNode(25.5, 15.2, 3600, "RFID_TAG_123");

  // Set auto reconnect for WiFi
  WiFi.setAutoReconnect(true);

  // Connect to WiFi
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

  // Initialize Google Sheets client
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop() {
  // Continuously check if Google Sheets client is ready
  if (GSheet.ready()) {
    if (!taskComplete) {
      FirebaseJson response;

      Serial.println("Appending data to spreadsheet...");
      Serial.println("---------------------------------------------------------------");

      Node* current = head;

      while (current != nullptr) {
        FirebaseJson valueRange;
        // FirebaseJson::Array valuesArray;

        // Create a timestamp for datetime (consider using NTP or RTC for accurate time)
        String datetime = String(millis() / 1000);  // Timestamp in seconds

        // Calculate hours, minutes, and seconds from tempoTotal
        int horaPercorrida = current->tempoTotal / 3600;
        int minutoPercorrido = (current->tempoTotal % 3600) / 60;
        int segundoPercorrido = current->tempoTotal % 60;

        // Add values to the array in the specified order
        valueRange.add("majorDimension", "COLUMNS");                  // datetime
        valueRange.set("values/[0]/[0]", datetime);  // velocidadeMedia
        valueRange.set("values/[1]/[0]", current->velocidadeMedia);  // velocidadeMedia
        valueRange.set("values/[2]/[0]", current->distanciaTotal);   // distanciaTotal
        valueRange.set("values/[3]/[0]", current->tempoTotal);       // tempoTotal
        valueRange.set("values/[4]/[0]", horaPercorrida);            // horaPercorrida
        valueRange.set("values/[5]/[0]", minutoPercorrido);          // minutoPercorrido
        valueRange.set("values/[6]/[0]", segundoPercorrido);         // segundoPercorrido
        valueRange.set("values/[7]/[0]", current->tagRFID);          // tagRFID

        // // Add the array to the "values" key
        // row.addArray("values", valuesArray);

        // Append data to the spreadsheet
        bool success = GSheet.values.append(&response, SPREADSHEET_ID, SHEET_NAME, &valueRange);

        if (success) {
          Serial.println("Data appended successfully:");
          response.toString(Serial, true);
        } else {
          Serial.println("Failed to append data.");
        }

        current = current->next;  // Move to the next node
      }

      taskComplete = true;
    }
  } else {
    Serial.println("Google Sheets client not ready.");
  }

  delay(1000);  // Adjust delay as needed
}