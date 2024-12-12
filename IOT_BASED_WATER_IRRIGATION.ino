#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Blynk and WiFi Configuration
#define BLYNK_PRINT Serial
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Pin Definitions
#define SOIL_MOISTURE_PIN A0
#define RELAY_PIN D1
#define PUMP_MOTOR_PIN D2

// Moisture Threshold Settings
#define MOISTURE_THRESHOLD 40  // Adjust based on soil type and plant needs

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 16, 2);  // I2C address 0x27, 16 column and 2 rows

// Blynk Virtual Pins
#define MOISTURE_VIRTUAL_PIN V5
#define PUMP_CONTROL_PIN V6

// Global Variables
int moistureLevel = 0;
bool isPumping = false;

void setup() {
  // Initialize Serial Communication
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("IoT Irrigation");
  lcd.setCursor(0,1);
  lcd.print("System");
  delay(2000);
  
  // Configure Pins
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(PUMP_MOTOR_PIN, OUTPUT);
  
  // Turn off pump initially
  digitalWrite(PUMP_MOTOR_PIN, LOW);
  
  // Connect to WiFi and Blynk
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Blynk.begin(BLYNK_AUTH_TOKEN, WIFI_SSID, WIFI_PASSWORD);
}

void loop() {
  // Run Blynk
  Blynk.run();
  
  // Read Soil Moisture
  moistureLevel = readMoisture();
  
  // Update Blynk with Moisture Level
  Blynk.virtualWrite(MOISTURE_VIRTUAL_PIN, moistureLevel);
  
  // Display on LCD
  updateLCD();
  
  // Check if Watering is Needed
  if (moistureLevel < MOISTURE_THRESHOLD) {
    startIrrigation();
  } else {
    stopIrrigation();
  }
  
  delay(5000);  // Check every 5 seconds
}

int readMoisture() {
  int rawValue = analogRead(SOIL_MOISTURE_PIN);
  // Convert raw sensor value to percentage
  // Calibrate these values based on your specific sensor
  int moisturePercentage = map(rawValue, 0, 1023, 0, 100);
  return moisturePercentage;
}

void startIrrigation() {
  if (!isPumping) {
    digitalWrite(PUMP_MOTOR_PIN, HIGH);
    digitalWrite(RELAY_PIN, HIGH);
    isPumping = true;
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Watering Started");
    
    Blynk.virtualWrite(PUMP_CONTROL_PIN, 1);
  }
}

void stopIrrigation() {
  if (isPumping) {
    digitalWrite(PUMP_MOTOR_PIN, LOW);
    digitalWrite(RELAY_PIN, LOW);
    isPumping = false;
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Watering Stopped");
    
    Blynk.virtualWrite(PUMP_CONTROL_PIN, 0);
  }
}

// Blynk Virtual Pin Handler for Manual Pump Control
BLYNK_WRITE(PUMP_CONTROL_PIN) {
  int pinValue = param.asInt();
  
  if (pinValue == 1) {
    startIrrigation();
  } else {
    stopIrrigation();
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Moisture: ");
  lcd.print(moistureLevel);
  lcd.print("%");
  
  lcd.setCursor(0,1);
  lcd.print(isPumping ? "Watering: ON" : "Watering: OFF");
}