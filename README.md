

🌱 Plantinha Inteligente: Monitoramento de Umidade do Solo com ESP32, LCD, Firebase e Blynk

📌 Descrição do Projeto:

O projeto Plantinha Inteligente foi desenvolvido com o objetivo de monitorar a umidade do solo de diferentes tipos de plantas, de forma automatizada e visual. Ele utiliza um sensor de umidade do solo, um display LCD I2C, conexão Wi-Fi via ESP32, e integração com a plataforma Firebase Realtime Database e o app Blynk IoT para controle e monitoramento remoto.

A ideia é permitir que o usuário possa acompanhar a saúde da planta localmente (via LCD e LED) e remotamente (pelo celular), podendo inclusive definir o tipo de planta e adaptar o sistema conforme suas necessidades hídricas específicas.

⚙️ Funcionalidades:

    🔍 Leitura periódica da umidade do solo
    💡 Display LCD 16x2 mostrando a umidade atual e o nome da planta selecionad
    🔴 LED indicador de alerta quando a planta está precisando de água
    ☁️ Envio automático dos dados para o Firebase, com timestamp e nome da planta
    📱 App Blynk para visualizar a umidade e escolher entre três tipos de plantas:

    Samambaia (muita água)

    Alecrim (água média)

    Cacto (pouca água)

🧠 Lógica de Rega (em breve):

Embora ainda não esteja ativo, o projeto está preparado para controle de uma bomba d'água via relé, que poderá ser acionada automaticamente quando a umidade estiver abaixo do ideal, conforme o tipo de planta definido.

🔧 Componentes Utilizados:

    ESP32 DevKit V1
    Sensor de umidade do solo (analógico)
    Display LCD 16x2 com interface I2C
    Módulo Relé (com suporte a relé de estado sólido)
    LED indicador
    Jumpers e fonte de alimentação
    App Blynk (iOS ou Android)
    Firebase Realtime Database (Google Cloud)

🧱 Tecnologias:

    C++ com Arduino (PlatformIO)
    FreeRTOS (nativamente no ESP32)
    Firebase ESP32 Client (Mobizt)
    Blynk IoT Platform
    LCD via LiquidCrystal_I2C
    Comunicação Wi-Fi com ESP32

🚀 Possibilidades de Expansão:

    Controle automático da irrigação com bomba d’água
    Integração com notificações via Blynk
    Histórico gráfico de umidade
    Conexão com sensores de temperatura, luminosidade ou pH
    Interface web (usando ESPAsyncWebServer ou Firebase Hosting)

🎯 Objetivo:

Facilitar o cuidado com plantas, unindo automação, conectividade e acessibilidade — ideal tanto para entusiastas de eletrônica quanto para quem simplesmente esquece de regar as plantinhas 🌿
