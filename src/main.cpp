#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>
#include <BlynkSimpleEsp32.h>

// =================== DEFINIÇÕES ====================
#define WIFI_SSID       "COS21"
#define WIFI_PASS       "Guireale2108"

#define DATABASE_URL    "https://plantinha2-0-default-rtdb.firebaseio.com/"
#define DATABASE_SECRET "0Ni86UbwQlNeIRYYiFScxhtdHImqGqEZ2LaQwAPy"

#define SENSOR_PIN      34
#define EXT_LED_PIN     2
#define I2C_SDA_PIN     21
#define I2C_SCL_PIN     22

const uint16_t DRY_VALUE = 4095;
const uint16_t WET_VALUE = 1800;
#define MOISTURE_THRESHOLD 3900

#define BLYNK_TEMPLATE_ID "TMPL2sOwqifSp"
#define BLYNK_TEMPLATE_NAME "Plantinha2"
#define BLYNK_AUTH_TOKEN "777MG_d3rR746qeA2pog88dIYoTz3EEM"

// =================== OBJETOS GLOBAIS ====================
LiquidCrystal_I2C lcd(0x27, 16, 2);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
BlynkTimer timer;

// =================== VARIÁVEIS ====================
unsigned long lastFirebaseUpdate = 0;
int soilMoisture = 0;
uint8_t currentPlantType = 1;  // valor padrão vindo do Blynk

struct Plant {
  const char* name;
  uint16_t moistureThreshold;
};

Plant plants[] = {
  {"Samambaia", 3600},   // tipo 1
  {"Bananeira", 3000},   // tipo 2
  {"Cacto",     4200},   // tipo 3
};

const size_t numPlants = sizeof(plants) / sizeof(plants[0]);

// =================== SETUP ====================
void setup() {
  Serial.begin(115200);

  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Conectando Wi-Fi");

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Wi-Fi conectado");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi conectado");

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.begin(&config, &auth);

  // Inicia Blynk e Timer
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASS);
  timer.setInterval(10000L, sendToBlynk);

  delay(1000);
  lcd.clear();
}

// =================== FUNÇÕES BLYNK ====================

// Envia umidade e tipo de planta atual para o app
void sendToBlynk() {
  int percent = map(soilMoisture, DRY_VALUE, WET_VALUE, 0, 100);
  percent = constrain(percent, 0, 100);

  Blynk.virtualWrite(V0, percent);           // Umidade
  Blynk.virtualWrite(V1, currentPlantType);  // Tipo atual
}

// Recebe o tipo de planta escolhido no app
BLYNK_WRITE(V1) {
  int val = param.asInt();
  if (val >= 1 && val <= numPlants) {
    currentPlantType = val;
    Serial.print("Tipo de planta via Blynk: ");
    Serial.println(plants[currentPlantType - 1].name);
  }
}

// =================== LOOP ====================
void loop() {
  soilMoisture = analogRead(SENSOR_PIN);
  int percent = map(soilMoisture, DRY_VALUE, WET_VALUE, 0, 100);
  percent = constrain(percent, 0, 100);

  // Seleciona planta válida
  size_t idx = (currentPlantType >= 1 && currentPlantType <= numPlants) ? currentPlantType - 1 : 0;
  const char* plantName = plants[idx].name;
  uint16_t threshold = plants[idx].moistureThreshold;

  // Atualiza LCD
  lcd.setCursor(0, 0);
  lcd.printf("%-10s", plantName);
  lcd.setCursor(0, 1);
  lcd.printf("Umidade:%3d%% (%4d)", percent, soilMoisture);

  // Liga LED se estiver seco
  digitalWrite(EXT_LED_PIN, soilMoisture > threshold ? HIGH : LOW);

  // Envia para Firebase a cada 10s
  if (millis() - lastFirebaseUpdate > 10000) {
    lastFirebaseUpdate = millis();

    if (Firebase.ready()) {
      String path = "/plantinha/leituras";
      FirebaseJson json;
      json.set("plantType", currentPlantType);
      json.set("plantName", plantName);
      json.set("umidade", percent);
      json.set("timestamp", millis());

      bool success = Firebase.pushJSON(fbdo, path.c_str(), json);
      Serial.printf("Firebase -> %s = %d%% [%s]\n", path.c_str(), percent,
                    success ? "OK" : fbdo.errorReason().c_str());
    }
  }

  Blynk.run();
  timer.run();

  delay(2000);
}
