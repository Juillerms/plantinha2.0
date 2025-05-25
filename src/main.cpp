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
#define WIFI_SSID "Casa Luz"  // Substitua pelo nome da sua rede
#define WIFI_PASS "1804199820"     // Substitua pela senha da sua rede

// --- Variáveis Globais ---
volatile uint16_t soil_moisture_value = 0;       // Valor lido do sensor (volatile para tasks)
LiquidCrystal_I2C  lcd(0x27, 16, 2);             // Endereço I2C do LCD, 16 colunas, 2 linhas

// --- Handles das Tasks do FreeRTOS ---
TaskHandle_t sensorTaskHandle     = NULL;
TaskHandle_t lcdTaskHandle        = NULL;
TaskHandle_t controlTaskHandle    = NULL;
TaskHandle_t uartCmdTaskHandle    = NULL;

//==============================================================================
// FUNÇÃO SETUP - Executada uma vez na inicialização
//==============================================================================
void setup() {
  // Inicializa a comunicação Serial (UART)
  Serial.println("\r\nIniciando o sistema...");

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
  // Inicializa os pinos GPIO
  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW); // Começa com o LED apagado

  // Inicializa o I2C e o LCD
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");


  Serial.println("Sistema iniciado e tarefas criadas.");

  // Conecta o ESP32 à rede Wi-Fi

}

//==============================================================================
// FUNÇÃO LOOP - No FreeRTOS, pode ser deixada vazia
//==============================================================================
void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}

