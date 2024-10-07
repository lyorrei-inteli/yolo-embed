#include "esp_camera.h"
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>  // Biblioteca para criar servidor web no ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

// Definição dos pinos para o ESP32-CAM (modelo AI Thinker)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM       5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
// Flash LED pin
#define FLASH_GPIO_NUM    4

// Configuração do WiFi
const char* ssid = "A";
const char* password = "12345678";
String apiEndpoint = "http://192.168.212.134:8000/run-model";

SemaphoreHandle_t xSemaphore = NULL;
camera_fb_t* fb = NULL;

WebServer server(80);  // Servidor web
uint8_t* receivedImageBuffer = NULL;  // Buffer para imagem processada
size_t receivedImageSize = 0;         // Tamanho da imagem processada

// Função para inicializar a câmera
void initializeCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // Resolução máxima UXGA
    config.jpeg_quality = 10;  // Alta qualidade
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // Resolução mais baixa para dispositivos sem PSRAM
    config.jpeg_quality = 12;  // Qualidade média
    config.fb_count = 1;
  }

  // Inicializa a câmera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Falha na inicialização da câmera: 0x%x\n", err);
    return;
  }

  Serial.println("Câmera inicializada com sucesso!");
}

// Página inicial do servidor web
void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>Imagem Processada</h1><img src='/image' /></body></html>");
}

// Rota para exibir a imagem processada
void handleImage() {
  if (receivedImageBuffer && receivedImageSize > 0) {
    server.sendHeader("Content-Type", "image/jpeg");
    server.send_P(200, "image/jpeg", (const char*)receivedImageBuffer, receivedImageSize);
  } else {
    server.send(404, "text/plain", "Imagem não encontrada");
  }
}

void initializeWebServer() {
  server.on("/", handleRoot);  
  server.on("/image", handleImage); 
  server.begin();
  Serial.println("Servidor web iniciado!");
}

// Thread de Aquisição de Imagens
void imageAcquisitionTask(void *pvParameters) {
  for (;;) {
    if (xSemaphoreTake(xSemaphore, portMAX_DELAY)) {
      fb = esp_camera_fb_get(); // Captura a imagem
      if (!fb) {
        Serial.println("Falha ao capturar imagem.");
      } else {
        Serial.println("Imagem capturada.");
      }
      xSemaphoreGive(xSemaphore);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

// Function to connect to WiFi
void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// Thread de Envio e Recebimento de Imagens
void imageSendingTask(void *pvParameters) {
  for (;;) {
    if (fb != NULL) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(apiEndpoint);
        http.addHeader("Content-Type", "image/jpeg");
        Serial.println("Sending image...");

        int httpResponseCode = http.POST(fb->buf, fb->len);
        if (httpResponseCode == 200) {
          Serial.printf("Imagem enviada: código de resposta %d\n", httpResponseCode);
          
          // Limpar buffer antigo, se houver
          if (receivedImageBuffer != NULL) {
            free(receivedImageBuffer);
            receivedImageBuffer = NULL;
          }

          // Obter a imagem processada da resposta
          receivedImageSize = http.getSize();
          if (receivedImageSize > 0) {
            receivedImageBuffer = (uint8_t*)malloc(receivedImageSize);
            if (receivedImageBuffer) {
              WiFiClient* stream = http.getStreamPtr();
              stream->readBytes(receivedImageBuffer, receivedImageSize);
              Serial.println("Imagem processada recebida com sucesso.");
            } else {
              Serial.println("Falha ao alocar memória para a imagem recebida.");
            }
          }
        } else {
          Serial.printf("Erro ao enviar a imagem. Código de resposta: %d\n", httpResponseCode);
        }
        http.end();
        esp_camera_fb_return(fb);
        fb = NULL;
      } else {
        Serial.println("WiFi desconectado.");
      }
    } else {
      Serial.println("Nenhuma imagem para enviar.");
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  initializeCamera();
  initializeWebServer();

  xSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(xSemaphore);

  xTaskCreate(imageAcquisitionTask, "Aquisição de Imagens", 8192, NULL, 1, NULL);
  xTaskCreate(imageSendingTask, "Envio e Recebimento de Imagens", 8192, NULL, 1, NULL);
}

void loop() {
  server.handleClient();
}
