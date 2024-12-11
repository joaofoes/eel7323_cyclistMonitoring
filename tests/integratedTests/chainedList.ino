#include <Arduino.h>
#include <vector> // Biblioteca para std::vector usada no gerenciamento da lista encadeada

// Estrutura TripData para armazenar os detalhes da viagem
struct TripData {
    float velocidadeMedia = 0.0f;
    float distanciaTotal = 0.0f;
    String tagRFID;
    String startDate;
    String startTime;
    String endDate;
    String endTime;
};

// Classe Node para representar os nós da lista encadeada
class Node {
public:
    TripData data;
    Node* next;

    Node(TripData tripData) : data(tripData), next(nullptr) {}
};

// Classe ChainedList para gerenciar operações na lista encadeada
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

    // Função para verificar se a lista está vazia
    bool isEmpty() {
        return head == nullptr;
    }
};

// Instância da lista encadeada
ChainedList tripList;

void setup() {
    // Inicializa o monitor serial
    Serial.begin(115200);
    delay(1000);
    Serial.println("Iniciando ESP32-S3 - Teste da Lista Encadeada");

    // Adiciona alguns registros fictícios
    tripList.addRecord({25.5f, 10.0f, "Tag123", "2024-08-01", "12:00", "2024-08-01", "12:30"});
    tripList.addRecord({30.2f, 15.5f, "Tag456", "2024-08-02", "13:00", "2024-08-02", "13:45"});
    tripList.addRecord({22.1f, 8.3f, "Tag789", "2024-08-03", "14:00", "2024-08-03", "14:45"});

    // Obtém e exibe todos os registros
    std::vector<TripData> records = tripList.getAllRecords();
    for (const auto& trip : records) {
        Serial.println("---------------------------");
        Serial.println("Velocidade Média: " + String(trip.velocidadeMedia) + " km/h");
        Serial.println("Distância Total: " + String(trip.distanciaTotal) + " km");
        Serial.println("Tag RFID: " + trip.tagRFID);
        Serial.println("Data Início: " + trip.startDate + " Hora: " + trip.startTime);
        Serial.println("Data Fim: " + trip.endDate + " Hora: " + trip.endTime);
    }

    tripList.clearRecords();
    // Verifica se a lista está vazia
    if (tripList.isEmpty()) {
        records.clear();
        Serial.println("A lista encadeada está vazia após a limpeza.");
    } else {
        Serial.println("A lista ainda contém registros.");
    }
}

void loop() {
    // Não há lógica contínua no loop() para esse exemplo
}
