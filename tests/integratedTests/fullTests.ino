#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <MFRC522.h>
#include <SPI.h>
#include <vector>

// Definições de pinos
#define RFID_SS_PIN 8
#define RFID_RST_PIN 21
#define OLED_CLOCK 4
#define OLED_DATA 5
#define GPS_RX 16
#define GPS_TX 17

// Instâncias de hardware serial
HardwareSerial gpsSerial(1);

// Estrutura TripData
struct TripData {
    float velocidadeMedia = 0.0f;
    float distanciaTotal = 0.0f;
    String tagRFID;
    String startDate;
    String startTime;
    String endDate;
    String endTime;
};

// Classes do sistema
class Node {
public:
    TripData data;
    Node* next;

    Node(TripData tripData) : data(tripData), next(nullptr) {}
};

class ChainedList {
private:
    Node* head;

public:
    ChainedList() : head(nullptr) {}

    ~ChainedList() {
        clearRecords();
    }

    void addRecord(TripData data) {
        Node* newNode = new Node(data);
        newNode->next = head;
        head = newNode;
    }

    std::vector<TripData> getAllRecords() {
        std::vector<TripData> records;
        Node* current = head;
        while (current != nullptr) {
            records.push_back(current->data);
            current = current->next;
        }
        return records;
    }

    void clearRecords() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
    }

    bool isEmpty() {
        return head == nullptr;
    }
};

class GPSModule {
private:
    TinyGPSPlus gps;
    HardwareSerial& serialGPS;
    float initialLat, initialLon;
    float totalDistance = 0.0f;
    unsigned long rideStartTime;

public:
    GPSModule(HardwareSerial& serial) : serialGPS(serial) {}

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
        }
    }

    void updateTripData() {
        if (gps.location.isUpdated() && gps.location.isValid()) {
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

class RFIDModule {
private:
    MFRC522 rfid;

public:
    RFIDModule(byte ssPin, byte rstPin) : rfid(RFID_SS_PIN, RFID_RST_PIN) {}

    void init() {
        SPI.begin(SCK, MISO, MOSI, RFID_SS_PIN);
        rfid.PCD_Init();
        Serial.println("RFID inicializado.");
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

class DisplayModule {
private:
    U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C display;

public:
    DisplayModule() : display(U8G2_R0, U8X8_PIN_NONE, OLED_CLOCK, OLED_DATA) {}

    void init() {
        display.begin();
        display.clearBuffer();
        display.setFont(u8g2_font_ncenB08_tr);
        Serial.println("Display inicializado.");
    }

    void showData(const String& data) {
        display.clearBuffer();
        display.drawStr(0, 12, data.c_str());
        display.sendBuffer();
        delay(2000);
    }
};

// Instâncias Globais
GPSModule gps(gpsSerial);
RFIDModule rfid(RFID_SS_PIN, RFID_RST_PIN);
DisplayModule display;
ChainedList tripList;

// Função de Teste GPS
void testGPSModule() {
    Serial.println("\n--- Testando GPS ---");
    gps.init();
    gps.captureData();
    Serial.println("\n--- Testando funcao startRide ---");
    gps.startRide();
    Serial.println("\n--- Testando funcao elapsedTime, esperar 5s: ---");
    delay(5000);
    Serial.println(gps.getTimeElapsed());
    Serial.println("\n--- Testando funcao updateTripData ---");
    gps.updateTripData();
    Serial.println("\n--- Testando funcao totalDistance ---");
    Serial.println("Distância Total: " + String(gps.getTotalDistance()) + " km");
    Serial.println("\n--- Testando funcoes de data");
    gps.captureData();
    Serial.println("Data: " + gps.getStartDate());
    Serial.println("Hora: " + gps.getStartTime());
    Serial.println("\n--- Testando funcoes de velocidade");
    Serial.println("Velocidade: " + String(gps.getSpeed()) + " km/h");
}

// Função de Teste RFID
void testRFIDModule() {
    Serial.println("\n--- Testando RFID ---");
    rfid.init();
    Serial.println("Aproxime uma tag RFID.");
    while (!rfid.isNewCardPresent()) {
        delay(100);
    }
    if (rfid.readCardSerial()) {
        String tag = rfid.identifyUser();
        Serial.println("Tag Identificada: " + tag);
    }
}

// Função de Teste Display
void testDisplayModule() {
    Serial.println("\n--- Testando Display ---");
    display.init();
    display.showData("Teste do Display");
}

// Função de Teste da Lista Encadeada
void testChainedList() {
    Serial.println("\n--- Testando Lista Encadeada ---");

    // Criar e adicionar registros fictícios
    TripData trip1 = {25.5f, 10.0f, "Tag123", "2024-08-01", "12:00", "2024-08-01", "12:30"};
    TripData trip2 = {30.2f, 15.5f, "Tag456", "2024-08-02", "13:00", "2024-08-02", "13:45"};

    tripList.addRecord(trip1);
    tripList.addRecord(trip2);

    // Teste: Obter registros
    std::vector<TripData> records = tripList.getAllRecords();
    for (const auto& trip : records) {
        Serial.println("Tag RFID: " + trip.tagRFID);
        Serial.println("Velocidade Média: " + String(trip.velocidadeMedia));
        Serial.println("Distância Total: " + String(trip.distanciaTotal));
    }

    // Limpar e testar lista vazia
    tripList.clearRecords();
    if (tripList.isEmpty()) {
        Serial.println("Lista Encadeada Limpa com Sucesso.");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Iniciando Testes...\n");

    testGPSModule();
    // testRFIDModule();
    // testDisplayModule();
    // testChainedList();

    // Serial.println("\nTodos os testes foram concluídos com sucesso!");
}

void loop() {
    // Nada a ser executado no loop
}
