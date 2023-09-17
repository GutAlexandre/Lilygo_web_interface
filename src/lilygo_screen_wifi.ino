#include "Wire.h"
#include "XL9535_driver.h"
#include "current_img.h"
#include "pin_config.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h> /* https://github.com/moononournation/Arduino_GFX.git */
#include <SPIFFS.h>

#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <esp_system.h>

String Image_str;
int Size;
int Sizepack;
const char* ssid = "Box-gut-2.4G";
const char* password = "Rut@b@g@93";
AsyncWebServer server(80);

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  -1, -1, -1, EXAMPLE_PIN_NUM_DE, EXAMPLE_PIN_NUM_VSYNC, EXAMPLE_PIN_NUM_HSYNC, EXAMPLE_PIN_NUM_PCLK,
  EXAMPLE_PIN_NUM_DATA1, EXAMPLE_PIN_NUM_DATA2, EXAMPLE_PIN_NUM_DATA3, EXAMPLE_PIN_NUM_DATA4, EXAMPLE_PIN_NUM_DATA5,
  EXAMPLE_PIN_NUM_DATA6, EXAMPLE_PIN_NUM_DATA7, EXAMPLE_PIN_NUM_DATA8, EXAMPLE_PIN_NUM_DATA9, EXAMPLE_PIN_NUM_DATA10, EXAMPLE_PIN_NUM_DATA11,
  EXAMPLE_PIN_NUM_DATA13, EXAMPLE_PIN_NUM_DATA14, EXAMPLE_PIN_NUM_DATA15, EXAMPLE_PIN_NUM_DATA16, EXAMPLE_PIN_NUM_DATA17);

Arduino_GFX *gfx = new Arduino_ST7701_RGBPanel(bus, GFX_NOT_DEFINED, 0 /* rotation */, false /* IPS */, 480, 480,
    st7701_type2_init_operations, sizeof(st7701_type2_init_operations), true,
    50, 1, 30, 20, 1, 30);

typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;



DRAM_ATTR static const lcd_init_cmd_t st_init_cmds[] = {
  {0xFF, {0x77, 0x01, 0x00, 0x00, 0x10}, 0x05},
  {0xC0, {0x3b, 0x00}, 0x02},
  {0xC1, {0x0b, 0x02}, 0x02},
  {0xC2, {0x07, 0x02}, 0x02},
  {0xCC, {0x10}, 0x01},
  {0xCD, {0x08}, 0x01}, // 用565时屏蔽    666打开
  {0xb0, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x06, 0x05, 0x09, 0x08, 0x21, 0x06, 0x13, 0x10, 0x29, 0x31, 0x18}, 0x10},
  {0xb1, {0x00, 0x11, 0x16, 0x0e, 0x11, 0x07, 0x05, 0x09, 0x09, 0x21, 0x05, 0x13, 0x11, 0x2a, 0x31, 0x18}, 0x10},
  {0xFF, {0x77, 0x01, 0x00, 0x00, 0x11}, 0x05},
  {0xb0, {0x6d}, 0x01},
  {0xb1, {0x37}, 0x01},
  {0xb2, {0x81}, 0x01},
  {0xb3, {0x80}, 0x01},
  {0xb5, {0x43}, 0x01},
  {0xb7, {0x85}, 0x01},
  {0xb8, {0x20}, 0x01},
  {0xc1, {0x78}, 0x01},
  {0xc2, {0x78}, 0x01},
  {0xc3, {0x8c}, 0x01},
  {0xd0, {0x88}, 0x01},
  {0xe0, {0x00, 0x00, 0x02}, 0x03},
  {0xe1, {0x03, 0xa0, 0x00, 0x00, 0x04, 0xa0, 0x00, 0x00, 0x00, 0x20, 0x20}, 0x0b},
  {0xe2, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x0d},
  {0xe3, {0x00, 0x00, 0x11, 0x00}, 0x04},
  {0xe4, {0x22, 0x00}, 0x02},
  {0xe5, {0x05, 0xec, 0xa0, 0xa0, 0x07, 0xee, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
  {0xe6, {0x00, 0x00, 0x11, 0x00}, 0x04},
  {0xe7, {0x22, 0x00}, 0x02},
  {0xe8, {0x06, 0xed, 0xa0, 0xa0, 0x08, 0xef, 0xa0, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, 0x10},
  {0xeb, {0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00}, 0x07},
  {0xed, {0xff, 0xff, 0xff, 0xba, 0x0a, 0xbf, 0x45, 0xff, 0xff, 0x54, 0xfb, 0xa0, 0xab, 0xff, 0xff, 0xff}, 0x10},
  {0xef, {0x10, 0x0d, 0x04, 0x08, 0x3f, 0x1f}, 0x06},
  {0xFF, {0x77, 0x01, 0x00, 0x00, 0x13}, 0x05},
  {0xef, {0x08}, 0x01},
  {0xFF, {0x77, 0x01, 0x00, 0x00, 0x00}, 0x05},
  {0x36, {0x08}, 0x01},
  {0x3a, {0x66}, 0x01},
  {0x11, {0x00}, 0x80},
  {0x29, {0x00}, 0x80},
  {0, {0}, 0xff}
};

XL9535 xl;
void tft_init(void);
void lcd_cmd(const uint8_t cmd);
void lcd_data(const uint8_t *data, int len);
void scan_iic(void) {
  byte error, address;
  int nDevices = 0;
  Serial.println("Scanning for I2C devices ...");
  for (address = 0x01; address < 0x7f; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.printf("I2C device found at address 0x%02X\n", address);
      nDevices++;
    } else if (error != 2) {
      Serial.printf("Error %d at address 0x%02X\n", error, address);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found");
  }
}
void setupSPIFFS() {
  if (SPIFFS.begin()) {
    Serial.println("Système de fichiers SPIFFS monté avec succès");
  } else {
    Serial.println("Erreur lors du montage du système de fichiers SPIFFS");
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connexion en cours au réseau Wi-Fi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nConnecté au réseau Wi-Fi");
  Serial.print("Adresse IP : ");
  Serial.println(WiFi.localIP());
}


uint16_t* convertStringToUint16Array(String inputString, int arraySize) {
  uint16_t* dataArray = new uint16_t[arraySize];
  int currentIndex = 0;
  int lastIndex = 0;
  for (int i = 0; i < inputString.length(); i++) {
    if (inputString.charAt(i) == ',' || i == inputString.length() - 1) {
      String token = inputString.substring(lastIndex, i + 1);
      lastIndex = i + 1;
      dataArray[currentIndex] = strtol(token.c_str(), NULL, 16);
      currentIndex++;
    }
  }
  return dataArray;
}


void setup() {
  pinMode(BAT_VOLT_PIN, ANALOG);
  Wire.begin(IIC_SDA_PIN, IIC_SCL_PIN, (uint32_t)800000);
  Serial.begin(115200);
  setupSPIFFS();
  connectToWiFi();
  xl.begin();
  uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);
  xl.pinMode8(0, pin, OUTPUT);
  xl.digitalWrite(PWR_EN_PIN, 1);
  pinMode(EXAMPLE_PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
  gfx->begin();
  tft_init();
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)image_data, 480, 480);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html");
  });
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest * request) {
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)image_data, 480, 480);
    request->send(200, "text/plain", "Image reset.");
  });
  server.on("/sendImageChunk", HTTP_POST, [](AsyncWebServerRequest * request) {
    if (!request->hasParam("imageChunk", true)) {
      request->send(400, "text/plain", "Erreur : aucun paquet reçu.");
      return;
    }
    String imageChunk = request->getParam("imageChunk", true)->value();
    //Serial.println(imageChunk);
    if (imageChunk.indexOf("Begin") != -1) {
      Image_str = "";
      int colonIndex1 = imageChunk.indexOf(':');
      int colonIndex2 = imageChunk.lastIndexOf(':');
      Size = imageChunk.substring(colonIndex1 + 2, colonIndex2).toInt(); // +2 pour ignorer l'espace après le premier ":"
      Sizepack = imageChunk.substring(colonIndex2 + 2).toInt();
      Serial.println("Debut de l'ecriture du fichier");
    } else if (imageChunk.indexOf("End") != -1 ) {
      //Serial.println(Image_str);
      uint16_t * data_image = convertStringToUint16Array(Image_str, Size);
      gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)data_image, 480, 480);
      Image_str = "";
      Serial.println("Fin de l'ecriture du fichier");
    } else {      
      if( imageChunk.length() == Sizepack ){ 
        Image_str.concat(imageChunk);
      }else{
        request->send(400, "text/plain", "Erreur Paquet !");
        return;
      }
      request->send(200, "text/plain", "Paquet reçu et ajouté !");
    }
    imageChunk = "";
  });
  server.begin();
}

void loop() {

}

void lcd_send_data(uint8_t data) {
  uint8_t n;
  for (n = 0; n < 8; n++) {
    if (data & 0x80)
      xl.digitalWrite(LCD_SDA_PIN, 1);
    else
      xl.digitalWrite(LCD_SDA_PIN, 0);

    data <<= 1;
    xl.digitalWrite(LCD_CLK_PIN, 0);
    xl.digitalWrite(LCD_CLK_PIN, 1);
  }
}

void lcd_cmd(const uint8_t cmd) {
  xl.digitalWrite(LCD_CS_PIN, 0);
  xl.digitalWrite(LCD_SDA_PIN, 0);
  xl.digitalWrite(LCD_CLK_PIN, 0);
  xl.digitalWrite(LCD_CLK_PIN, 1);
  lcd_send_data(cmd);
  xl.digitalWrite(LCD_CS_PIN, 1);
}

void lcd_data(const uint8_t *data, int len) {
  uint32_t i = 0;
  if (len == 0)
    return; // no need to send anything
  do {
    xl.digitalWrite(LCD_CS_PIN, 0);
    xl.digitalWrite(LCD_SDA_PIN, 1);
    xl.digitalWrite(LCD_CLK_PIN, 0);
    xl.digitalWrite(LCD_CLK_PIN, 1);
    lcd_send_data(*(data + i));
    xl.digitalWrite(LCD_CS_PIN, 1);
    i++;
  } while (len--);
}

void tft_init(void) {
  xl.digitalWrite(LCD_CS_PIN, 1);
  xl.digitalWrite(LCD_SDA_PIN, 1);
  xl.digitalWrite(LCD_CLK_PIN, 1);
  xl.digitalWrite(LCD_RST_PIN, 1);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xl.digitalWrite(LCD_RST_PIN, 0);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  xl.digitalWrite(LCD_RST_PIN, 1);
  vTaskDelay(200 / portTICK_PERIOD_MS);
  int cmd = 0;
  while (st_init_cmds[cmd].databytes != 0xff) {
    lcd_cmd(st_init_cmds[cmd].cmd);
    lcd_data(st_init_cmds[cmd].data, st_init_cmds[cmd].databytes & 0x1F);
    if (st_init_cmds[cmd].databytes & 0x80) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    cmd++;
  }
  Serial.println("Register setup complete");
}
