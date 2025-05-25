/**
 * @file    main.cpp
 * @brief   Projeto Plantinha com LCD I2C para ESP32 e conexão Wi-Fi.
 * @version 1.1
 * @date    2025-05-25
 * @note    Adaptado de um projeto original para STM32.
 * Utiliza FreeRTOS para gerenciar tarefas de leitura do sensor,
 * atualização do LCD, controle de LED e interpretação de comandos via UART.
 */

// Includes principais do framework Arduino, Wi-Fi e FreeRTOS
#include <Arduino.h>
#include <Wire.h>              // Para comunicação I2C
#include <LiquidCrystal_I2C.h> // Biblioteca do LCD I2C
#include <WiFi.h>              // Biblioteca Wi-Fi do ESP32


// --- Definições de Hardware e Pinos (Ajuste conforme sua montagem) ---

// Sensor de Umidade do Solo (Pino ADC)
#define SENSOR_PIN      34  // Pinos ADC1 no ESP32: 32, 33, 34, 35, 36, 39

// LED Externo
#define EXT_LED_PIN     2   // Escolha qualquer pino GPIO disponível

// Pinos I2C para o LCD
#define I2C_SDA_PIN     21  // Pino padrão para SDA
#define I2C_SCL_PIN     22  // Pino padrão para SCL


#define FIREBASE_URL "https://plantinha2-0-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "0Ni86UbwQlNeIRYYiFScxhtdHImqGqEZ2LaQwAPy"

// --- Constantes do Projeto ---
#define MOISTURE_THRESHOLD 3900   // Valor ADC para solo muito seco
#define UART_BAUD_RATE    115200  // Taxa de comunicação serial

// Calibração do Sensor (ajuste conforme suas medições)
const uint16_t DRY_VALUE = 4095;  // Valor ADC com o sensor no ar (0% umidade)
const uint16_t WET_VALUE = 1800;  // Valor ADC com o sensor na água (100% umidade)

// --- Configurações de";    
#define WIFI_SSID "COS21"  // Substitua pelo nome da sua rede
#define WIFI_PASS "Guirealle2108"     // Substitua pela senha da sua rede

// --- Variáveis Globais ---
volatile uint16_t soil_moisture_value = 0;       // Valor lido do sensor (volatile para tasks)
LiquidCrystal_I2C  lcd(0x27, 16, 2);             // Endereço I2C do LCD, 16 colunas, 2 linhas

// --- Handles das Tasks do FreeRTOS ---
TaskHandle_t sensorTaskHandle     = NULL;
TaskHandle_t lcdTaskHandle        = NULL;
TaskHandle_t controlTaskHandle    = NULL;
TaskHandle_t uartCmdTaskHandle    = NULL;

// --- Protótipos das Funções (Tasks) ---
void startSensorTask(void *pvParameters);
void startLcdTask(void *pvParameters);
void startControlTask(void *pvParameters);
void startUARTCommandTask(void *pvParameters);
void processUARTCommand(char *cmd);

// Protótipo da função de conexão Wi-Fi
void connectToWiFi();

//==============================================================================
// FUNÇÃO SETUP - Executada uma vez na inicialização
//==============================================================================
void setup() {
  // Inicializa a comunicação Serial (UART)
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
      Serial.print(".");
      delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.begin(UART_BAUD_RATE);
  Serial.println("\r\nIniciando o sistema...");

  // Inicializa os pinos GPIO
  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW); // Começa com o LED apagado

  // Inicializa o I2C e o LCD
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Cria as tarefas do FreeRTOS
  xTaskCreate(startSensorTask,      "SensorTask", 2048, NULL, 1, &sensorTaskHandle);
  xTaskCreate(startLcdTask,         "LcdTask",    4096, NULL, 1, &lcdTaskHandle);
  xTaskCreate(startControlTask,     "ControlTask",1024, NULL, 1, &controlTaskHandle);
  xTaskCreate(startUARTCommandTask, "UartCmdTask",2048, NULL, 1, &uartCmdTaskHandle);

  Serial.println("Sistema iniciado e tarefas criadas.");

  // Conecta o ESP32 à rede Wi-Fi

}

//==============================================================================
// FUNÇÃO LOOP - No FreeRTOS, pode ser deixada vazia
//==============================================================================
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

//==============================================================================
// FUNÇÃO connectToWiFi
//==============================================================================  
void connectToWiFi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin("COS21", "Guireale2108");

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi conectado com sucesso!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar ao Wi-Fi.");
  }
}

//==============================================================================
// IMPLEMENTAÇÃO DAS TAREFAS
//==============================================================================

void startSensorTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    soil_moisture_value = analogRead(SENSOR_PIN);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void startLcdTask(void *pvParameters) {
  (void)pvParameters;
  char msg_line1[17];
  char msg_line2[17];

  for (;;) {
    uint16_t current_value = soil_moisture_value;
    int moisture_percent = map(current_value, DRY_VALUE, WET_VALUE, 0, 100);
    moisture_percent = constrain(moisture_percent, 0, 100);

    if (moisture_percent >= 60) {
      strcpy(msg_line1, "Estou hidratada,");
      sprintf(msg_line2, "obrigado!   %3d%%", moisture_percent);
    } else {
      strcpy(msg_line1, "Estou com sede,");
      sprintf(msg_line2, "me hidrate! %3d%%", moisture_percent);
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(msg_line1);
    lcd.setCursor(0, 1);
    lcd.print(msg_line2);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void startControlTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (soil_moisture_value > MOISTURE_THRESHOLD) {
      digitalWrite(EXT_LED_PIN, HIGH);
    } else {
      digitalWrite(EXT_LED_PIN, LOW);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void startUARTCommandTask(void *pvParameters) {
  (void)pvParameters;
  char rxBuffer[50];
  int index = 0;

  Serial.println("UART Command Task Iniciada. Digite 'GET SENSOR'.");

  for (;;) {
    if (Serial.available() > 0) {
      char ch = Serial.read();
      if (ch == '\r' || ch == '\n') {
        if (index > 0) {
          rxBuffer[index] = '\0';
          Serial.print("\r\nComando recebido: ");
          Serial.println(rxBuffer);
          processUARTCommand(rxBuffer);
          index = 0;
        }
      } else if (index < sizeof(rxBuffer) - 1) {
        rxBuffer[index++] = ch;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void processUARTCommand(char *cmd) {
  char response[100];
  for (int i = 0; cmd[i]; i++) {
    cmd[i] = toupper(cmd[i]);
  }

  if (strcmp(cmd, "GET SENSOR") == 0) {
    sprintf(response, "Valor Sensor: %u\r\n", soil_moisture_value);
  } else {
    sprintf(response, "Comando desconhecido: %s\r\n", cmd);
  }
  Serial.print(response);
}
