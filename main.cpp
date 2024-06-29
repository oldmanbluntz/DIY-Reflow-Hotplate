#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi credentials
const char* ssid = "SSID_HERE";
const char* password = "PASSWORD_HERE";

// Define pin constants for SD card and OLED display
#define SD_CS 5
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Define pin constants for LEDs and button
const int ledPinYellow = 2, ledPinBlue = 13, ledPinGreen = 14, ledPinRed = 12, button = 26;

// Initialize web server and sensor/display objects
WebServer server(80);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Global variables
bool SD_present = false;
String fileName = "/curve.txt";
File dataCurve;
float targetTemp = 0, temp = 0;
int ledStatus = LOW, start = 0, digitalButton = 0, firstTime = 1;
unsigned long currentMillis = 0, lastButtonPress = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  // Initialize pins
  pinMode(ledPinBlue, OUTPUT); pinMode(ledPinGreen, OUTPUT); pinMode(ledPinYellow, OUTPUT); pinMode(ledPinRed, OUTPUT); pinMode(button, INPUT_PULLUP);
  digitalWrite(ledPinGreen, LOW); digitalWrite(ledPinBlue, LOW); digitalWrite(ledPinYellow, LOW); digitalWrite(ledPinRed, LOW);
  checkLeds(); // Initialize LED sequence

  mlx.begin(); // Initialize temperature sensor
  delay(500);

  // Initialize SD card
  if (!SD.begin(SD_CS)) SD_present = false; else {
    SD_present = true;
    if (SD.exists(fileName)) digitalWrite(ledPinGreen, HIGH); else digitalWrite(ledPinGreen, LOW);
  }

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) return;

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(1000);

  // Configure server routes
  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/script.js", SPIFFS, "/script.js");
  server.on("/upload", HTTP_POST, []() { server.send(200, "text/plain", "File Uploaded"); }, handleFileUpload);
  server.on("/start", HTTP_POST, []() { digitalButton = 1; server.sendHeader("Location", "/"); server.send(303); });
  server.on("/stop", HTTP_POST, []() { digitalButton = 0; stopHeatingCycle(); server.sendHeader("Location", "/"); server.send(303); });
  server.on("/ledstatus", HTTP_GET, []() {
    StaticJsonDocument<200> doc;
    doc["ledBlue"] = digitalRead(ledPinBlue);
    doc["ledGreen"] = digitalRead(ledPinGreen);
    doc["ledYellow"] = digitalRead(ledPinYellow);
    doc["ledRed"] = digitalRead(ledPinRed);
    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
  });
  server.begin(); // Start server

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) for(;;);
  display.display(); delay(2000);
  display.clearDisplay(); display.setTextSize(2); display.setTextColor(SSD1306_WHITE);
}

void loop() {
  server.handleClient(); // Handle client requests
  temp = mlx.readObjectTempC(); // Read temperature from sensor

  // Update OLED display
  display.clearDisplay();
  display.setCursor(0, 0); display.print("Temp:"); display.setCursor(0, 16); display.print(temp); display.print(" C");
  display.setCursor(0, 32); display.print("Expected:"); display.setCursor(0, 48); display.print(targetTemp); display.print(" C");
  display.display();

  // Update red LED based on temperature
  if (temp > 40) digitalWrite(ledPinRed, HIGH); else digitalWrite(ledPinRed, LOW);

  // Update green LED based on presence of curve file
  if (SD_present) {
    if (SD.exists(fileName)) digitalWrite(ledPinGreen, HIGH); else digitalWrite(ledPinGreen, LOW);
  }

  // Handle button press or start command
  if (start == 0) {
    unsigned long currentMillis = millis();
    if ((digitalRead(button) == LOW && (currentMillis - lastButtonPress > debounceDelay)) || digitalButton == 1) {
      lastButtonPress = currentMillis; start = 1; digitalButton = 0; currentMillis = millis(); firstTime = 1;
    }
  }

  // Handle heating cycle
  if (start == 1) {
    if (firstTime == 1) {
      firstTime = 0; dataCurve = SD.open(fileName);
    }

    if (!dataCurve) {
      checkTemp();
    } else {
      String row = dataCurve.readStringUntil('\n');
      if (row.indexOf("e") >= 0) { } else {
        targetTemp = row.toFloat(); digitalWrite(ledPinGreen, HIGH);
      }
    }

    if (temp < targetTemp - 1) ledStatus = HIGH; else ledStatus = LOW;
    digitalWrite(ledPinYellow, ledStatus);
    delay(250);
  }
}

// LED sequence function
void checkLeds() {
  digitalWrite(ledPinBlue, HIGH); delay(300); digitalWrite(ledPinGreen, HIGH); delay(300); digitalWrite(ledPinYellow, HIGH); delay(300); digitalWrite(ledPinRed, HIGH);
  delay(300); digitalWrite(ledPinRed, LOW); delay(300); digitalWrite(ledPinYellow, LOW); delay(300); digitalWrite(ledPinGreen, LOW); delay(1000);
}

// Default temperature curve
void checkTemp() {
  if (millis() <= 75000 + currentMillis) targetTemp = 2.00 * (millis() - currentMillis) / 1000.00; else if (millis() <= 155000 + currentMillis) targetTemp = 150;
  else if (millis() <= 190000 + currentMillis) targetTemp = 150.00 + 2.00 * (millis() - 155000 - currentMillis) / 1000.00; else if (millis() <= 230000 + currentMillis) targetTemp = 220;
  else targetTemp = 220;
}

// Handle file upload
void handleFileUpload() {
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) filename = "/" + filename;
    dataCurve = SD.open(filename, FILE_WRITE);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (dataCurve) dataCurve.write(upload.buf, upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (dataCurve) {
      dataCurve.close(); server.sendHeader("Location", "/"); server.send(303);
    } else server.send(500, "text/plain", "Failed to save the file");
  }
}

// Stop heating cycle
void stopHeatingCycle() {
  start = 0; digitalButton = 0; targetTemp = 0; digitalWrite(ledPinGreen, LOW); digitalWrite(ledPinYellow, LOW); digitalWrite(ledPinRed, LOW);
}
