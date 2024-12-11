#include <TinyGPSPlus.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <MFRC522.h>
#include <SPI.h>
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


#define RFID_SS_PIN 8
#define RFID_RST_PIN 21
#define OLED_CLOCK 4
#define OLED_DATA 5
#define GPS_RX 16
#define GPS_TX 17

bool startRide = false;
bool wifiConnected = false;

// estrutura TripData para armazenar os detalhes da viagem
struct TripData {
    float velocidadeMedia = 0.0f;
    float distanciaTotal = 0.0f;
    String tagRFID;
    String startDate;
    String startTime;
    String endDate;
    String endTime;
};

// classe Node para a lista encadeada
class Node {
public:
    TripData data;
    Node* next;

    Node(TripData tripData) : data(tripData), next(nullptr) {}
};

// classe ChainedList para gerenciar operações na lista encadeada
class ChainedList {
private:
    Node* head;

public:
    ChainedList() : head(nullptr) {}

    ~ChainedList() {
        clearRecords();
    }

    // Função para adicionar um registro
    void addRecord(TripData data) {
        Node* newNode = new Node(data);
        newNode->next = head;
        head = newNode;
    }

    // Função para retornar todos os registros como um vetor
    std::vector<TripData> getAllRecords() {
        std::vector<TripData> records; // Vetor para armazenar os dados
        Node* current = head;          // Ponteiro temporário para percorrer a lista

        // Percorre a lista encadeada
        while (current != nullptr) {
            records.push_back(current->data);
            current = current->next;
        }

        return records; // Retorna o vetor com todos os registros
    }

    // Função para limpar todos os registros
    void clearRecords() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }
};

// classe base Sensor
class Sensor {
public:
    virtual void init() = 0;
    virtual void captureData() = 0;
};

// classe GPSModule herdando de Sensor
class GPSModule : public Sensor {
private:
    TinyGPSPlus gps;
    HardwareSerial& serialGPS; // usar referência
    float initialLat, initialLon;
    float totalDistance;
    unsigned long rideStartTime = 0;

public:
    GPSModule(HardwareSerial& serial) : serialGPS(serial), totalDistance(0), rideStartTime(0) {}

    void init() {
        serialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
    }

    void captureData() {
        while (serialGPS.available() > 0) {
            gps.encode(serialGPS.read());
        }
    }

    void startRide() {
        if (gps.location.isValid()) {
            initialLat = gps.location.lat();
            initialLon = gps.location.lng();
            totalDistance = 0;  // redefinir a distância total
            rideStartTime = millis();  // registrar o tempo de início
        } else {
          rideStartTime = millis();
        }
    }

    void updateTripData() {
        if (gps.location.isValid()) {
            float currentLat = gps.location.lat();
            float currentLon = gps.location.lng();
            totalDistance += gps.distanceBetween(initialLat, initialLon, currentLat, currentLon); // uso correto

            // atualizar a posição inicial para a posição atual
            initialLat = currentLat;
            initialLon = currentLon;
        }
    }

    float getSpeed() {
        while (serialGPS.available() > 0) {
            gps.encode(serialGPS.read());
        }

        if (gps.speed.isValid()){
          return gps.speed.kmph();  // velocidade em km/h
        } else {
          return -1.0f;
        }
    }

    float getTotalDistance() {
        return totalDistance;  // distância total em metros
    }

    int getTimeElapsed() {
        // retorna o tempo decorrido em segundos
        return (millis() - rideStartTime) / 1000;  // tempo em segundos
    }

    String getStartDate() {
        while (serialGPS.available() > 0) {
            gps.encode(serialGPS.read());
        }

        if (gps.date.isValid()) {
            return String(gps.date.year()) + "-" + String(gps.date.month()) + "-" + String(gps.date.day());
        } else {
            return "data inválida";
        }
    }

    String getStartTime() {
        if (gps.time.isValid()) {
            Serial.println("Hora válida:");
            return String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second());
        } else {
            return "Hora invalida";
        }
    }
};

// classe RFIDModule herdando de Sensor
class RFIDModule : public Sensor {
private:
    MFRC522 rfid;

public:
    RFIDModule(byte ssPin, byte rstPin) : rfid(RFID_SS_PIN, RFID_RST_PIN) {}

    void init() override {
        SPI.begin(SCK, MISO, MOSI, RFID_SS_PIN);
        rfid.PCD_Init();
    }

    void captureData() override {

    }

    bool isNewCardPresent() {
        return rfid.PICC_IsNewCardPresent();
    }

    bool readCardSerial() {
        return rfid.PICC_ReadCardSerial();
    }

    String identifyUser() {
        String tag = "";
        for (byte i = 0; i < rfid.uid.size; i++) {
            tag += String(rfid.uid.uidByte[i], HEX);
        }
        tag.toUpperCase();
        return tag;
    }
};

// classe DisplayModule para gerenciar a exibição OLED
class DisplayModule {
private:
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display;

public:
    DisplayModule() : display(U8G2_R0, U8X8_PIN_NONE, OLED_CLOCK, OLED_DATA) {} // ajustar o pino de reset, se necessário

    void init() {
        display.begin();
        display.clearBuffer();
        display.setFont(u8g2_font_ncenB08_tr); // definir uma fonte adequada
    }

    void showData(const String& data) {
        display.clearBuffer();
        int lineHeight = 10; // ajustar com base no tamanho da fonte
        int y = lineHeight;

        int start = 0;
        int end = data.indexOf('\n');
        while (end != -1) {
            String line = data.substring(start, end);
            display.drawStr(0, y, line.c_str());
            y += lineHeight;
            start = end + 1;
            end = data.indexOf('\n', start); // It searches for a \n in the next line on the string, if it doesn't find, it will attribute
            // -1 to the end variable, locking out of the loop.
        }
        // desenhar a última linha
        String line = data.substring(start);
        display.drawStr(0, y, line.c_str());

        display.sendBuffer();
    }
};

class GoogleSheetsSender {
public:
    GoogleSheetsSender() {
      GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
    }

    // bool wifiInit() {
    //   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    //   Serial.print("Connecting to Wi-Fi");
    //   display.showData("Conectando na \n wiFi...");
    //   delay(1000);
    //   unsigned long startAttemptTime = millis();
    //   while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 50000) {
    //     Serial.print(".");
    //     delay(300);
    //   }

    //   if (WiFi.status() == WL_CONNECTED) {
    //     Serial.println();
    //     Serial.print("Connected with IP: ");
    //     display.showData("Wifi conectada.");
    //     delay(1000);
    //     Serial.println(WiFi.localIP());
    //     wifiConnected = true;
    //     return true;
    //   } else {
    //     Serial.println();
    //     Serial.println("Failed to connect to Wi-Fi.");
    //     display.showData("Falha na conexao.");
    //     delay(1000);
    //     wifiConnected = false;
    //     return false;
    //   }
    // }

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

// classe CyclistMonitor para lógica principal de monitoramento
class CyclistMonitor {
private:
    GPSModule gps;
    RFIDModule rfid;
    DisplayModule display;
    ChainedList tripRecords;
    GoogleSheetsSender sheetSender;
    TripData trip;
    bool rideStarted = false;
    String currentTag = "";

public:
    CyclistMonitor(HardwareSerial& gpsSerial, byte rfidSSPin, byte rfidRSTPin)
        : gps(gpsSerial), rfid(rfidSSPin, rfidRSTPin) {}

    void init() {
        gps.init();
        rfid.init();
        display.init();
    }

    void startMonitoring() {
        display.showData("Aproxime RFID \npara iniciar...");
    }

    bool waitForRFID(bool wifiConnected) {
      if (wifiConnected){
        if (rfid.isNewCardPresent() && rfid.readCardSerial()) {
            String userTag = rfid.identifyUser();

            // Caso a tag seja a mesma e a viagem já tenha começado
            if (rideStarted && userTag.equals(currentTag)) {
                Serial.println(userTag);
                Serial.println(currentTag);
                String endDate = gps.getStartDate();
                String endTime = gps.getStartTime();

                // Atualiza os dados da viagem com a data/hora de término
                trip.endDate = endDate;
                trip.endTime = endTime;

                trip.distanciaTotal = gps.getTotalDistance();
                int tempoTotal = gps.getTimeElapsed();
                trip.velocidadeMedia = trip.distanciaTotal / tempoTotal;

                display.showData("Viagem encerrada");
                storeTripData(trip); // Invoca a função de processamento
                sheetSender.sendTripData(trip);
                tripRecords.clearRecords();
                // Resetar os valores para aguardar nova viagem
                rideStarted = false;
                currentTag = "";
                delay(3000);
                return rideStarted;
            } 
            // Caso a tag seja lida pela primeira vez
            else if (!rideStarted) {
                display.showData("Viagem iniciada");
                delay(1000);

                // Inicializa dados da viagem
                trip.tagRFID = userTag;
                trip.startDate = gps.getStartDate();
                trip.startTime = gps.getStartTime();

                // Salva a tag atual e inicia o rastreamento
                currentTag = userTag;
                rideStarted = true;
                gps.startRide(); // Inicia rastreamento do GPS
                return rideStarted;
            } else {
              return rideStarted;
            }
        } else {
          return rideStarted;
        }
      } else {
        if (rfid.isNewCardPresent() && rfid.readCardSerial()) {
            String userTag = rfid.identifyUser();

            // Caso a tag seja a mesma e a viagem já tenha começado
            if (rideStarted && userTag.equals(currentTag)) {
                String endDate = gps.getStartDate();
                String endTime = gps.getStartTime();

                // Atualiza os dados da viagem com a data/hora de término
                trip.endDate = endDate;
                trip.endTime = endTime;

                trip.distanciaTotal = gps.getTotalDistance();
                int tempoTotal = gps.getTimeElapsed();
                trip.velocidadeMedia = trip.distanciaTotal / tempoTotal;

                display.showData("Viagem encerrada");
                storeTripData(trip); // Invoca a função de processamento
                
                // A aplicacao da funcao wifiInit foi desativada devido a problemas relacionados ao FreeRTOS que impossibilitava a completa funcionalidade da função proposta.
                // if (sheetSender.wifiInit()) {
                //   sheetSender.sendTripData(trip);
                // } 

                // Resetar os valores para aguardar nova viagem
                rideStarted = false;
                currentTag = "";
                delay(3000);
                return rideStarted;
            } 
            // Caso a tag seja lida pela primeira vez
            else if (!rideStarted) {
                display.showData("Viagem iniciada");
                delay(1000);

                // Inicializa dados da viagem
                trip.tagRFID = userTag;
                trip.startDate = gps.getStartDate();
                trip.startTime = gps.getStartTime();

                // Salva a tag atual e inicia o rastreamento
                currentTag = userTag;
                rideStarted = true;
                gps.startRide(); // Inicia rastreamento do GPS
                return rideStarted;
            } else {
              return rideStarted;
            }
        } else {
          return rideStarted;
        }
      }        
    }

    void processSensorData() {
        if (rideStarted) {
            gps.captureData();
            // rfid.captureData(); This function doesn't exist, the only captureData available is the one in GPS Class.
            gps.updateTripData();  // atualizar distância e tempo
        }
    }

    void calculateTripData() {
        if (rideStarted) {
            // obter a distância total em metros
            float totalDistance = gps.getTotalDistance();

            // obter o tempo decorrido em segundos
            int elapsedTime = gps.getTimeElapsed();

            // calcular a velocidade em km/h
            float speed = gps.getSpeed();

            // atualizar os dados da viagem (apenas um exemplo, pode ser expandido conforme necessário)
            String displayData = "dist: " + String(totalDistance, 2) + " m\n";
            displayData += "tempo: " + String(elapsedTime) + " s\n"; // elapsedTime is necessary only to display to user.
            displayData += "velocidade: " + String(speed, 2) + " km/h";

            // exibir os dados na tela OLED
            display.showData(displayData);
        }
    }

    void storeTripData(TripData data) {
        tripRecords.addRecord(data);
    }
};

// inicializar CyclistMonitor com pinos RFID atualizados para evitar conflito no I2C
CyclistMonitor monitor(Serial1, SDA, 16); // assumindo GPS no Serial1, RFID SS no GPIO5, RST no GPIO17
DisplayModule display;

void setup() {

  Serial.begin(115200);
  Serial.println("Starting...");
  WiFi.setAutoReconnect(true);

  display.init();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  display.showData("Conectando na \n wiFi...");
  delay(1000);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    Serial.print(".");
    delay(300);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("Connected with IP: ");
    display.showData("WiFi conectada.");
    delay(1000);
    Serial.println(WiFi.localIP());
    wifiConnected = true;
  } else {
    Serial.println();
    Serial.println("Failed to connect to Wi-Fi.");
    display.showData("Falha na conexão.");
    delay(2500);
    wifiConnected = false;
  }
  
  monitor.init();
  monitor.startMonitoring();
}

void loop() {
  startRide = monitor.waitForRFID(wifiConnected);  // aguardar pelo RFID
  if (startRide) {
    monitor.processSensorData();      // capturar dados dos sensores
    monitor.calculateTripData();      // processar e exibir dados da viagem
    delay(500);                       // adicionar um pequeno atraso para evitar alto uso da CPU
  } else {
    monitor.startMonitoring();
  } 
}
