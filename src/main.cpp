#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <FirebaseESP32.h>
#include <addons/RTDBHelper.h>

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

// ================ OBJETOS GLOBAIS ==================
LiquidCrystal_I2C lcd(0x27, 16, 2);
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ================== VARIÁVEIS ======================
unsigned long lastFirebaseUpdate = 0;
const uint8_t plantType = 1;  // <-- ajuste aqui
int soilMoisture = 0;

struct Plant {
  const char* name;
  uint16_t moistureThreshold;
};

Plant plants[] = {
  {"Samambaia", 3600},   // tipo 1
  {"Bananeira", 3000},   // tipo 2
  {"Cacto",     4200},   // tipo 3
  // {"Outra",    XXXX},  // tipo 4, etc.
};
const size_t numPlants = sizeof(plants)/sizeof(plants[0]);

// ================== SETUP ==========================
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

  delay(1000);
  lcd.clear();
}

// ================= LOOP ============================
void loop() {
  // Leitura do sensor
  soilMoisture = analogRead(SENSOR_PIN);
  int percent = map(soilMoisture, DRY_VALUE, WET_VALUE, 0, 100);
  percent = constrain(percent, 0, 100);

  // Seleciona planta (evita índice fora do array)
  size_t idx = (plantType >= 1 && plantType <= numPlants) ? plantType - 1 : 0;
  const char* plantName = plants[idx].name;
  uint16_t threshold = plants[idx].moistureThreshold;

  // Atualiza LCD
  lcd.setCursor(0, 0);
  lcd.printf("%-10s", plantName);
  lcd.setCursor(0, 1);
  lcd.printf("Umidade:%3d%% (%4d)", percent, soilMoisture);

  // Liga LED se estiver seco
  digitalWrite(EXT_LED_PIN, soilMoisture > MOISTURE_THRESHOLD ? HIGH : LOW);

  // Envia para o Firebase a cada 10 segundos
  if (millis() - lastFirebaseUpdate > 10000) {
    lastFirebaseUpdate = millis();

    if (Firebase.ready()) {
      String path = "/plantinha/leituras";
      FirebaseJson json;
      json.set("plantType", plantType);
      json.set("plantName", plantName);
      json.set("umidade", percent);
      json.set("timestamp", millis()); // ou usar RTC para data real
          
      bool success = Firebase.pushJSON(fbdo, path.c_str(), json);
      Serial.printf("Firebase -> %s = %d [%s]\n", path.c_str(), percent,
                    success ? "OK" : fbdo.errorReason().c_str());
    }
  }

  delay(2000);
}
