#include "Wire.h"
#include "XL9535_driver.h"
#include "current_img.h"
#include "blanc.h"
#include "pin_config.h"
#include "lvgl.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h> /* https://github.com/moononournation/Arduino_GFX.git */
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebSrv.h>
#include <esp_system.h>
#include "OneButton.h"

#include "FreeMono8pt7b.h"
#include "FreeSansBold10pt7b.h"
#include "FreeSerifBoldItalic12pt7b.h"

#define USING_2_1_INC_CST820     1 

#define MSG_BAT_VOLT_UPDATE 1
#define MSG_TOUCH_UPDATE    2
#define MSG_WIFI_UPDATE     3
#define MSG_TOUCH_INT_UPDATE     4
#define TOUCH_MODULES_CST_SELF
#include "TouchLib.h"

TouchLib touch(Wire, IIC_SDA_PIN, IIC_SCL_PIN, CTS820_SLAVE_ADDRESS, TP_RES_PIN);


struct FileInfo {
  char fileName[255];
  size_t fileSize;
};
typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes; // No of data in data; bit 7 = delay after set; 0xFF = end of cmds.
} lcd_init_cmd_t;

String network;
uint16_t * data_image;
String lastimg;
const int MAX_FILES = 100;
FileInfo fileInfos[MAX_FILES];
int fileCount = 0;
String Image_str;
int Size;
int Sizepack;
String ssid , password ;
int status = 0 ;
String statut = "0";
bool click = false;
const int backlightPin = EXAMPLE_PIN_NUM_BK_LIGHT;
TaskHandle_t pvCreatedTask;
String message_to_write = "";

AsyncWebServer server(80);

Arduino_ESP32RGBPanel *bus = new Arduino_ESP32RGBPanel(
  -1, -1, -1, EXAMPLE_PIN_NUM_DE, EXAMPLE_PIN_NUM_VSYNC, EXAMPLE_PIN_NUM_HSYNC, EXAMPLE_PIN_NUM_PCLK,
  EXAMPLE_PIN_NUM_DATA1, EXAMPLE_PIN_NUM_DATA2, EXAMPLE_PIN_NUM_DATA3, EXAMPLE_PIN_NUM_DATA4, EXAMPLE_PIN_NUM_DATA5,
  EXAMPLE_PIN_NUM_DATA6, EXAMPLE_PIN_NUM_DATA7, EXAMPLE_PIN_NUM_DATA8, EXAMPLE_PIN_NUM_DATA9, EXAMPLE_PIN_NUM_DATA10, EXAMPLE_PIN_NUM_DATA11,
  EXAMPLE_PIN_NUM_DATA13, EXAMPLE_PIN_NUM_DATA14, EXAMPLE_PIN_NUM_DATA15, EXAMPLE_PIN_NUM_DATA16, EXAMPLE_PIN_NUM_DATA17);

Arduino_GFX *gfx = new Arduino_ST7701_RGBPanel(bus, GFX_NOT_DEFINED, 0 /* rotation */, false /* IPS */, 480, 480,
    st7701_type2_init_operations, sizeof(st7701_type2_init_operations), true,
    50, 1, 30, 20, 1, 30);



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
OneButton button(0, true);


void tft_init(void);
void lcd_cmd(const uint8_t cmd);
void lcd_data(const uint8_t *data, int len);
void deep_sleep(void);
bool touchDevicesOnline = false;
uint8_t touchAddress = 0;

typedef struct {
  uint16_t x;
  uint16_t y;
} touch_point_t;

static void lv_touchpad_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data) {
  static uint16_t lastX, lastY;
  touch_point_t p = {0};
  if (touch.read()) {
    TP_Point t = touch.getPoint(0);
    data->point.x = p.x = t.x;
    data->point.y = p.y = t.y;
    data->state = LV_INDEV_STATE_PR;
  } else {
    data->state = LV_INDEV_STATE_REL;
  }
  lv_msg_send(MSG_TOUCH_UPDATE, &p);
}

void waitInterruptReady() {
  Serial.println("Click");
  uint32_t timeout = millis() + 500;
  while (timeout > millis()) {
    while (!digitalRead(TP_INT_PIN)) {
      delay(20);
      timeout = millis() + 500;
    }
  }
  delay(10);
}



void scanDevices(void) {
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

void ecriture(String fichiername, String value) {
  File dataFile = SPIFFS.open(fichiername, "w");
  if (!dataFile) {
    Serial.println("Erreur lors de l'ouverture du fichier en écriture.");
  } else {
    if (dataFile.print(value)) {
      Serial.println("Écriture dans le fichier réussie.");
    } else {
      Serial.println("Erreur lors de l'écriture dans le fichier.");
    }
    dataFile.close();
  }
}


void append(String fichiername, String value) {
  File dataFile = SPIFFS.open(fichiername, "a");
  if (!dataFile) {
    Serial.println("Erreur lors de l'ouverture du fichier en mode ajout.");
  } else {
    if (dataFile.print(value)) {
      Serial.println("Ajout au fichier réussi.");
    } else {
      Serial.println("Erreur lors de l'ajout au fichier.");
    }
    dataFile.close();
  }
}

void deleteFile(fs::FS &fs, String path) {
  if (fs.exists(path)) {
    Serial.println("- Fichier existe");
    if (fs.remove(path)) {
      Serial.println("- Fichier supprimé avec succès");
    } else {
      Serial.println("- Échec de la suppression du fichier");
    }
  } else {
    Serial.println("- Le fichier " + path + " n'existe pas");
  }
}


String lecture(String fichiername) {
  String result = "";
  File dataFile = SPIFFS.open(fichiername, "r");
  if (!dataFile) {
    Serial.println("Erreur lors de l'ouverture du fichier SPIFFS");
    return "";
  }
  for (int i = 0; i < dataFile.size() ; i++) {
    result = result + (char)dataFile.read();
  }
  dataFile.close();
  return (result);
}

bool deleteAndCreateEmptyFile(const String &filename) {
  if (!SPIFFS.begin()) {
    Serial.println("Erreur lors de l'initialisation de SPIFFS");
    return false;
  }
  String newFilename = "/"; // Le nouveau nom de fichier
  int lastDotIndex = filename.lastIndexOf(".");

  if (lastDotIndex != -1) {
    newFilename += filename.substring(0, lastDotIndex);
  } else {
    newFilename += filename;
  }
  newFilename += ".txt";

  if (SPIFFS.exists(newFilename)) {
    if (SPIFFS.remove(newFilename)) {
      Serial.println("Fichier supprimé avec succès");
    } else {
      Serial.println("Erreur lors de la suppression du fichier");
      return false;
    }
  }

  File file = SPIFFS.open(newFilename, "w");
  if (!file) {
    Serial.println("Erreur lors de la création du fichier");
    return false;
  } else {
    Serial.println("Fichier créé avec succès");
    file.close();
    return true;
  }
}


FileInfo* listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
  fileCount = 0;

  File root = fs.open(dirname);
  if (!root) {
    return nullptr;
  }
  if (!root.isDirectory()) {
    return nullptr;
  }
  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      if (fileCount < MAX_FILES) {
        strcpy(fileInfos[fileCount].fileName, file.name());
        fileInfos[fileCount].fileSize = file.size();
        fileCount++;
      }
    }
    file = root.openNextFile();
  }
  return fileInfos;
}


String loadHtmlFromSpiffs(const char* path) {
  File file = SPIFFS.open(path, "r");
  if (!file) {
    Serial.println("Erreur lors de l'ouverture du fichier SPIFFS");
    return "";
  }
  String html = file.readString();
  file.close();
  return html;
}

String connectToWiFi() {
  ssid = lecture("/ssid.txt");
  password = lecture("/wep.txt");
  WiFi.begin(ssid.c_str(), password.c_str());
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 20) ) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (retries > 19) {
    Serial.println(F("WiFi connection FAILED"));
    statut = "Failed";
    return (statut);
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnecte au reseau Wi-Fi");
    Serial.print("Adresse IP : ");
    Serial.println(WiFi.localIP());
    network = "Wifi";
  }
  statut = "Success";
  return (statut);
}

void accespoint() {
  ssid = "Screen_wifi";
  const char* pass = NULL;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), pass);
  Serial.print("[+] AP Created with IP Gateway : ");
  Serial.println(WiFi.softAPIP());
  network = "AP";
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

String uint16ArrayToString(uint16_t* array, size_t length) {
  String result = "";
  for (size_t i = 0; i < length; i++) {
    result += String(array[i]);
    // Ajoutez un séparateur si nécessaire (par exemple, une virgule)
    if (i < length - 1) {
      result += ",";
    }
  }
  return result;
}

void setup() {

  pinMode(BAT_VOLT_PIN, ANALOG);
  Wire.begin(IIC_SDA_PIN, IIC_SCL_PIN, (uint32_t)800000);
  Serial.begin(115200);
  setupSPIFFS();
  statut = connectToWiFi();
  if (statut != "Success" ) {
    accespoint();
  }
  button.attachClick([]() {
    click = true;
  });
  xl.begin();
  uint8_t pin = (1 << PWR_EN_PIN) | (1 << LCD_CS_PIN) | (1 << TP_RES_PIN) | (1 << LCD_SDA_PIN) | (1 << LCD_CLK_PIN) |
                (1 << LCD_RST_PIN) | (1 << SD_CS_PIN);
  xl.pinMode8(0, pin, OUTPUT);
  xl.digitalWrite(PWR_EN_PIN, 1);
  pinMode(EXAMPLE_PIN_NUM_BK_LIGHT, OUTPUT);
  digitalWrite(EXAMPLE_PIN_NUM_BK_LIGHT, EXAMPLE_LCD_BK_LIGHT_ON_LEVEL);
  gfx->begin();
  tft_init();
  lastimg = "image_data";
  gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)image_data, 480, 480);
 
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {//affiche la page de push
    request->send(SPIFFS, "/index.html");
  });

  server.on("/sendimg", HTTP_GET, [](AsyncWebServerRequest * request) {//affiche la page de push
    request->send(SPIFFS, "/sendimg.html");
  });
  
  server.on("/setwifi", HTTP_GET, [](AsyncWebServerRequest * request) { //affiche la page de changement d'identifiant de connection
    request->send(SPIFFS, "/Connection.html");
  });

  server.on("/removeimg", HTTP_POST, [](AsyncWebServerRequest * request) { // delete l'image du spiffs
    String filename = request->getParam("filename", true)->value();
    deleteFile(SPIFFS, "/" + filename);
    request->send(200, "text/plain", "Image remove.");
  });


  server.on("/imglist", HTTP_GET, [](AsyncWebServerRequest * request) { //affiche toutes les images dans le spiffs
    FileInfo* files = listDir(SPIFFS, "/", 0);
    String html = loadHtmlFromSpiffs("/img.html"); // Chargez le modèle HTML
    String fileList = "<ul>";
    bool foundHeaderFile = false;
    for (int i = 0; i < fileCount; i++) {
      String fileName = String(fileInfos[i].fileName);
      if (fileName.endsWith(".txt") && (fileName != "ssid.txt" && fileName != "wep.txt")) {
        fileList += "<li>";
        fileList += "<button id=\"" + fileName + "\" onclick=\"buttonClick(this)\">";
        fileList += "File Name: " + fileName;
        fileList += "</button>";
        fileList += "<button onclick=\"removeFile('" + fileName + "')\">&#10006;</button>"; // Bouton avec une croix
        fileList += "<br>";
        fileList += "File Size: " + String(fileInfos[i].fileSize) + " bytes";
        fileList += "</li>";
        foundHeaderFile = true;
      }
    }
    fileList += "</ul>";
    if (!foundHeaderFile) {
      fileList = "Aucun fichiers";
    }
    html.replace("{{FILE_LIST}}", fileList);
    request->send(200, "text/html", html);
  });

  server.on("/putimg", HTTP_POST, [](AsyncWebServerRequest * request) {
    String img = request->getParam("img", true)->value();
    int lastDotIndex = img.lastIndexOf(".");
    String newFilename = "/";
    if (lastDotIndex != -1) {
      newFilename += img.substring(0, lastDotIndex);
    } else {
      newFilename += img;
    }
    newFilename += ".txt";
    String Image_str = lecture(newFilename);

    uint16_t * data_image = convertStringToUint16Array(Image_str, Size);
    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)data_image, 480, 480);
    Serial.println(Image_str);
    delete [] data_image;
    Image_str = "";
  });
  server.on("/senddata", HTTP_POST, [](AsyncWebServerRequest * request) {
    String SSID = request->getParam("SSID", true)->value();
    String WPA = request->getParam("WPA", true)->value();
    ecriture("/ssid.txt", SSID);
    ecriture("/wep.txt", WPA);
    Serial.println("Redemarrage de l'esp ...");
    ESP.restart();
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
    int save = request->getParam("savestatus", true)->value().toInt();
    String filename = request->getParam("filename", true)->value();
    String imageChunk = request->getParam("imageChunk", true)->value();
    int indexchunck = request->getParam("actual", true)->value().toInt();;
    int targetchunck = request->getParam("target", true)->value().toInt();;

    if (imageChunk.indexOf("Begin") != -1) {
      message_to_write = "";
      Image_str = "";
      Size = imageChunk.substring(imageChunk.indexOf(':') + 2, imageChunk.lastIndexOf(':')).toInt(); // +2 pour ignorer l'espace après le premier ":"
      Sizepack = imageChunk.substring(imageChunk.lastIndexOf(':') + 2).toInt();
      Serial.println("Debut de l'ecriture du fichier");
    } else if (imageChunk.indexOf("End") != -1 ) {
      data_image = convertStringToUint16Array(Image_str, Size);
      gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)data_image, 480, 480);
      lastimg = "data_image";
      delay(1000);
      Image_str = "";
      Serial.println("Fin de l'ecriture du fichier");
    } else {
      if ( imageChunk.length() == Sizepack ) {
        Image_str.concat(imageChunk);
        gfx->setFont(&FreeSerifBoldItalic12pt7b);
        gfx->setTextColor(GREEN);
        int range = ((28 * indexchunck) / targetchunck);
        message_to_write = "";
        for (int i = 0; i < range ; i++) {
          message_to_write += "=";
        }
        
        gfx->setTextColor(GREEN);
        gfx->setCursor(45, (480 / 2) - 2);
        gfx->println(message_to_write);
        gfx->setCursor(45, (480 / 2));
        gfx->println(message_to_write);
        gfx->setCursor(45, (480 / 2) + 2);
        gfx->println(message_to_write);
        gfx->setCursor(40, (480 / 2) - 2);
        gfx->println(message_to_write);
        gfx->setCursor(40, (480 / 2));
        gfx->println(message_to_write);
        gfx->setCursor(40, (480 / 2) + 2);
        gfx->println(message_to_write);
      } else {
        request->send(400, "text/plain", "Erreur Paquet !");
        return;
      }
      request->send(200, "text/plain", "Paquet reçu et ajouté !");
    }
    imageChunk = "";
  });
  server.begin();
}



bool lastStatus = false;

void loop() {

  static uint32_t Millis;
  delay(2);
  lv_timer_handler();
  if (millis() - Millis > 50) {
    float v = (analogRead(BAT_VOLT_PIN) * 2 * 3.3) / 4096;
    lv_msg_send(MSG_BAT_VOLT_UPDATE, &v);
    Millis = millis();
  }
  bool touched = digitalRead(TP_INT_PIN) == LOW;
  if (touched) {
    lastStatus = true;
    String message_to_write;
    int length_of_string ;
    int i = 0;
    gfx->setFont(&FreeSerifBoldItalic12pt7b);
    gfx->setTextColor(RED);

    gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)blanc, 480, 480);
    if (network == "AP") {
      message_to_write = "[+] Connecte en mode point d'acces : ";
      length_of_string = message_to_write.length();
      gfx->setCursor((480 / 2) - (12 * (length_of_string / 2)), (480 / 2) - (7 / 2));
      gfx->println(message_to_write);
      message_to_write =  WiFi.softAPIP().toString();
      length_of_string = message_to_write.length();
      gfx->setCursor((480 / 2) - (12 * (length_of_string / 2)), (480 / 2) - (7 / 2) + 20);
      gfx->println(message_to_write);
    } else if (network == "Wifi") {
      message_to_write = "[+] Connecte au reseau Wi-Fi : ";
      length_of_string = message_to_write.length();
      gfx->setCursor((480 / 2) - (12 * (length_of_string / 2)), (480 / 2) - (7 / 2));
      gfx->println(message_to_write);
      message_to_write = WiFi.localIP().toString();
      length_of_string = message_to_write.length();
      gfx->setCursor((480 / 2) - (12 * (length_of_string / 2)), (480 / 2) - (7 / 2) + 20);
      gfx->println(message_to_write);
    } else {
      message_to_write = "IDK";
      length_of_string = message_to_write.length();
    }

    delay(10000);
  } else if (!touched && lastStatus) {
    lastStatus = false;

    if (lastimg == "image_data") {
      gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)image_data, 480, 480);
    } else {
      gfx->draw16bitRGBBitmap(0, 0, (uint16_t *)data_image, 480, 480);
    }

  }
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



void setBrightness(uint8_t value)
{
  static uint8_t level = 0;
  static uint8_t steps = 16;
  if (value == 0) {
    digitalWrite(backlightPin, 0);
    delay(3);
    level = 0;
    return;
  }
  if (level == 0) {
    digitalWrite(backlightPin, 1);
    level = steps;
    delayMicroseconds(30);
  }
  int from = steps - level;
  int to = steps - value;
  int num = (steps + to - from) % steps;
  for (int i = 0; i < num; i++) {
    digitalWrite(backlightPin, 0);
    digitalWrite(backlightPin, 1);
  }
  level = value;
}

void deep_sleep(void) {
  Serial.print("deep_sleep");
  if (pvCreatedTask) {
    vTaskDelete(pvCreatedTask);
  }
  WiFi.disconnect();
  pinMode(TP_INT_PIN, INPUT);
  waitInterruptReady();
  delay(2000);
  for (int i = 16; i >= 0; --i) {
    setBrightness(i);
    delay(30);
  }
  waitInterruptReady();
  delay(1000);

  esp_sleep_enable_ext1_wakeup(1ULL << TP_INT_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
  esp_deep_sleep_start();
}
