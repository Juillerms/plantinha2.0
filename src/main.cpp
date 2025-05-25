/**
 * @file    main.cpp
 * @brief   Projeto Plantinha com LCD I2C para ESP32.
 * @version 1.0
 * @date    2025-05-23
 * * @note    Adaptado de um projeto original para STM32.
 * Utiliza FreeRTOS para gerenciar tarefas de leitura do sensor,
 * atualização do LCD, controle de LED e interpretação de comandos via UART.
 */

// Includes principais do framework Arduino e FreeRTOS
#include <Arduino.h>
#include <Wire.h> // Para comunicação I2C
#include <LiquidCrystal_I2C.h> // Biblioteca do LCD I2C

// --- Definições de Hardware e Pinos (Ajuste conforme sua montagem) ---

// Sensor de Umidade do Solo (Pino ADC)
#define SENSOR_PIN 34 // Pinos ADC1 no ESP32: 32, 33, 34, 35, 36, 39

// LED Externo
#define EXT_LED_PIN 2 // Escolha qualquer pino GPIO disponível

// Pinos I2C para o LCD
#define I2C_SDA_PIN 21 // Pino padrão para SDA
#define I2C_SCL_PIN 22 // Pino padrão para SCL

// --- Constantes do Projeto ---
#define MOISTURE_THRESHOLD 3900  // Valor ADC para solo muito seco (maior valor = mais seco)
#define UART_BAUD_RATE 115200    // Taxa de comunicação serial

// Calibração do Sensor (ajuste conforme suas medições)
const uint16_t DRY_VALUE = 4095; // Valor ADC com o sensor no ar (0% umidade)
const uint16_t WET_VALUE = 1800; // Valor ADC com o sensor na água (100% umidade)

// --- Variáveis Globais ---
volatile uint16_t soil_moisture_value = 0; // Valor lido do sensor (volatile para acesso seguro entre tasks)
LiquidCrystal_I2C lcd(0x27, 16, 2); // Endereço I2C do LCD (pode ser 0x3F), 16 colunas, 2 linhas

// --- Handles das Tarefas (Tasks) do FreeRTOS ---
TaskHandle_t sensorTaskHandle = NULL;
TaskHandle_t lcdTaskHandle = NULL;
TaskHandle_t controlTaskHandle = NULL;
TaskHandle_t uartCmdTaskHandle = NULL;

// --- Protótipos das Funções (Tarefas) ---
void startSensorTask(void *pvParameters);
void startLcdTask(void *pvParameters);
void startControlTask(void *pvParameters);
void startUARTCommandTask(void *pvParameters);
void processUARTCommand(char *cmd);

//==============================================================================
// FUNÇÃO SETUP - Executada uma vez na inicialização
//==============================================================================
void setup() {
  // Inicializa a comunicação Serial (UART)
  Serial.begin(UART_BAUD_RATE);
  Serial.println("\r\nIniciando o sistema...");

  // Inicializa os pinos GPIO
  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW); // Começa com o LED apagado

  // O pino do sensor (ADC) é configurado automaticamente pela função analogRead()

  // Inicializa o I2C e o LCD
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Cria as tarefas do FreeRTOS
  // xTaskCreate(função, "nome", stack_size, parâmetros, prioridade, handle)
  xTaskCreate(startSensorTask, "SensorTask", 2048, NULL, 1, &sensorTaskHandle);
  xTaskCreate(startLcdTask, "LcdTask", 4096, NULL, 1, &lcdTaskHandle); // LCD e sprintf usam mais stack
  xTaskCreate(startControlTask, "ControlTask", 1024, NULL, 1, &controlTaskHandle);
  xTaskCreate(startUARTCommandTask, "UartCmdTask", 2048, NULL, 1, &uartCmdTaskHandle);
  
  Serial.println("Sistema iniciado e tarefas criadas.");
}

//==============================================================================
// FUNÇÃO LOOP - No modo FreeRTOS, pode ser deixada vazia
//==============================================================================
void loop() {
  // O scheduler do FreeRTOS gerencia as tarefas.
  // A função loop() também roda como uma tarefa de baixa prioridade.
  // Deixá-la vazia ou com um pequeno delay é uma prática comum.
  vTaskDelay(pdMS_TO_TICKS(1000));
}

//==============================================================================
// IMPLEMENTAÇÃO DAS TAREFAS
//==============================================================================

/**
 * @brief Task para leitura do sensor de umidade via ADC.
 * Lê a cada 1 segundo e atualiza a variável global.
 */
void startSensorTask(void *pvParameters) {
  (void)pvParameters; // Evita aviso de parâmetro não utilizado
  for (;;) { // Loop infinito da tarefa
    // A resolução padrão do ADC do ESP32 é 12 bits (0-4095)
    soil_moisture_value = analogRead(SENSOR_PIN);
    
    // Pausa a tarefa por 1000 milissegundos
    // vTaskDelay é a forma correta de fazer delays em tarefas no FreeRTOS
    // pdMS_TO_TICKS converte milissegundos para "ticks" do sistema
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

/**
 * @brief Task para atualização do display LCD com base na umidade.
 */
void startLcdTask(void *pvParameters) {
  (void)pvParameters;
  char msg_line1[17];
  char msg_line2[17];
  
  for (;;) {
    uint16_t current_value = soil_moisture_value;
    
    // Mapeia o valor do sensor para uma porcentagem (0-100%)
    // A lógica é invertida: maior valor ADC = mais seco
    int moisture_percent = map(current_value, DRY_VALUE, WET_VALUE, 0, 100);

    // Garante que a porcentagem fique entre 0 e 100
    if (moisture_percent < 0) moisture_percent = 0;
    if (moisture_percent > 100) moisture_percent = 100;
    
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

/**
 * @brief Task para controle do LED externo.
 * Acende o LED se o solo estiver muito seco.
 */
void startControlTask(void *pvParameters) {
  (void)pvParameters;
  for (;;) {
    if (soil_moisture_value > MOISTURE_THRESHOLD) {
      digitalWrite(EXT_LED_PIN, HIGH); // Acende o LED
    } else {
      digitalWrite(EXT_LED_PIN, LOW); // Apaga o LED
    }
    vTaskDelay(pdMS_TO_TICKS(100)); // Verifica 10 vezes por segundo
  }
}

/**
 * @brief Task para interpretar comandos recebidos via UART.
 */
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
          rxBuffer[index] = '\0'; // Finaliza a string
          Serial.print("\r\nComando recebido: ");
          Serial.println(rxBuffer);
          processUARTCommand(rxBuffer);
          index = 0; // Reseta para o próximo comando
        }
      } else if (index < sizeof(rxBuffer) - 1) {
        rxBuffer[index++] = ch;
      }
    }
    // Pequeno delay para não sobrecarregar a CPU
    vTaskDelay(pdMS_TO_TICKS(20)); 
  }
}

/**
 * @brief Processa os comandos recebidos via UART.
 */
void processUARTCommand(char *cmd) {
  char response[100];

  // Converte o comando para maiúsculas para facilitar a comparação
  for(int i = 0; cmd[i]; i++){
    cmd[i] = toupper(cmd[i]);
  }

  if (strcmp(cmd, "GET SENSOR") == 0) {
    sprintf(response, "Valor Sensor: %u\r\n", soil_moisture_value);
    Serial.print(response);
  } else {
    sprintf(response, "Comando desconhecido: %s\r\n", cmd);
    Serial.print(response);
  }
}