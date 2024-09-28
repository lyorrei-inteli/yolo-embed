#include "esp_camera.h"
#include "Arduino.h"
#include <WiFi.h>
#include <HTTPClient.h>

// Define the pins for the ESP32-CAM (AI Thinker model)
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

// Variable to enable/disable flash
bool flashEnabled = false; // Set to 'true' to enable flash, 'false' to disable

// WiFi configuration
const char* ssid = "A";
const char* password = "12345678";

// API endpoint
String apiEndpoint = "http://192.168.99.135:8000/run-model"; // Replace with your API address

// Function to initialize the camera
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

  // Set frame size and quality based on available PSRAM
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;  // Maximum resolution UXGA
    config.jpeg_quality = 10;  // High quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;  // Lower resolution for devices without PSRAM
    config.jpeg_quality = 12;  // Medium quality
    config.fb_count = 1;
  }

  // Initialize the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera initialization failed: 0x%x\n", err);
    return;
  }

  Serial.println("Camera initialized successfully!");
}

// Function to configure the flash pin
void initializeFlash() {
  pinMode(FLASH_GPIO_NUM, OUTPUT);
  Serial.println("Flash pin configured.");
}

// Function to capture an image
camera_fb_t* captureImage() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Failed to capture image.");
    return NULL;
  }

  Serial.printf("Image captured: Size = %u bytes, Width = %u px, Height = %u px\n", fb->len, fb->width, fb->height);
  return fb;
}

// Function to release the image
void releaseImage(camera_fb_t* fb) {
  if (fb) {
    esp_camera_fb_return(fb);
    Serial.println("Image buffer released.");
  }
}

// Function to turn on the flash
void turnOnFlash() {
  if (flashEnabled) {
    digitalWrite(FLASH_GPIO_NUM, HIGH);
    Serial.println("Flash turned on.");
  }
}

// Function to turn off the flash
void turnOffFlash() {
  if (flashEnabled) {
    digitalWrite(FLASH_GPIO_NUM, LOW);
    Serial.println("Flash turned off.");
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

// Function to send the image as raw binary data
void sendImageToAPI(camera_fb_t *fb) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Open connection to API endpoint
    http.begin(apiEndpoint);
    
    // Set the Content-Type to image/jpeg
    http.addHeader("Content-Type", "image/jpeg");

    // Send the image buffer as raw binary data
    int httpResponseCode = http.POST(fb->buf, fb->len);

    // Check server response
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.printf("API Response: %s\n", response.c_str());
    } else {
      Serial.printf("Failed to send image: %d\n", httpResponseCode);
    }

    // Close connection
    http.end();
  } else {
    Serial.println("Error: WiFi is not connected.");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Connect to WiFi
  connectToWiFi();

  // Initialize the camera
  initializeCamera();

  // Initialize flash pin
  initializeFlash();
}

void loop() {
  // Turn on flash before capturing image
  turnOnFlash();
  delay(100);  // Small delay to allow flash to light up

  // Capture the image
  camera_fb_t *fb = captureImage();

  // Turn off flash after capturing
  turnOffFlash();

  if (fb) {
    // Send the captured image to the API
    sendImageToAPI(fb);

    // Release image buffer
    releaseImage(fb);
  }

  // Delay before capturing the next image
  delay(5000);  // Wait 5 seconds before capturing and sending the next image
}
