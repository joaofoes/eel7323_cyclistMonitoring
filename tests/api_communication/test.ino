#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Google_Sheet_Client.h>
// #include <FirebaseJson.h>  // Ensure this library is included

// WiFi Credentials
#define WIFI_SSID 
#define WIFI_PASSWORD 

// Google Cloud Project Details
#define PROJECT_ID 

// Service Account Credentials
#define CLIENT_EMAIL 

// Service Account's Private Key (ensure proper formatting with \n)

// Spreadsheet Details
#define SPREADSHEET_ID 
#define SHEET_NAME 

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