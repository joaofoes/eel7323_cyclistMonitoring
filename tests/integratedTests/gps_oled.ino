#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <U8g2lib.h>

// Instância do TinyGPS++
TinyGPSPlus gps;

// UART para o módulo GPS
HardwareSerial SerialGPS(1); // UART1 do ESP32-S3

// Pinos do GPS
#define RXD2 16 // RX conectado ao TX do GPS
#define TXD2 17 // TX conectado ao RX do GPS

// Configuração do display OLED (I2C)
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 4, /* data=*/ 5);

// Variáveis para controle de tempo
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000; // Atualizar a cada 3 segundos

void setup() {
  // Inicializa comunicação serial para debug
  Serial.begin(115200);
  Serial.println("Inicializando GPS e Display OLED...");

  // Inicializa UART para o GPS
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Inicializa o display OLED
  u8g2.begin();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr); // Fonte simples e pequena
  u8g2.drawStr(0, 10, "Iniciando...");
  u8g2.sendBuffer();
}

void loop() {
  // Ler dados do GPS
  while (SerialGPS.available() > 0) {
    char c = SerialGPS.read();
    gps.encode(c); // Decodifica os dados do GPS
  }

  // Atualiza o display a cada 3 segundos
  if (millis() - lastUpdate >= updateInterval) {
    lastUpdate = millis();

    // Limpa o buffer do display
    u8g2.clearBuffer();

    // Obtém hora (UTC) do GPS
    if (gps.time.isValid()) {
      char timeBuffer[16];
      sprintf(timeBuffer, "Hora: %02d:%02d:%02d", gps.time.hour(), gps.time.minute(), gps.time.second());
      u8g2.drawStr(0, 12, timeBuffer); // Exibe a hora no display
    } else {
      u8g2.drawStr(0, 12, "Hora: --:--:--");
    }

    // Obtém velocidade do GPS
    if (gps.speed.isValid()) {
      char speedBuffer[16];
      sprintf(speedBuffer, "Vel: %.2f km/h", gps.speed.kmph());
      u8g2.drawStr(0, 28, speedBuffer); // Exibe a velocidade no display
    } else {
      u8g2.drawStr(0, 28, "Vel: 0.00 km/h");
    }

    // Envia os dados para o display
    u8g2.sendBuffer();
  }
}
