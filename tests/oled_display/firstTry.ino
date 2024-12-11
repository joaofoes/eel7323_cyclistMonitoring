#include <U8g2lib.h>
#include <Wire.h>

U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 4, /* data=*/ 5, /* reset=*/ U8X8_PIN_NONE);

void setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_unifont_t_symbols); // Fonte com suporte a muitos símbolos
}

void loop() {
  u8g2.clearBuffer();
  
  u8g2.setCursor(0, 15);
  u8g2.print("Temp: 25"); 
  u8g2.print((char)176); // Símbolo de grau
  
  u8g2.setCursor(0, 30);
  u8g2.print("Sinal: ");
  u8g2.print((char)9728); // Símbolo de estrela
  
  u8g2.sendBuffer();  // Envia o buffer para o display
  delay(2000);
}
