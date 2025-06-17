#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_BMP280.h>
#include <BlynkSimpleEsp32.h>
// Blynk credentials
#define BLYNK_TEMPLATE_ID "TMPL3EcZvOI0s"
#define BLYNK_TEMPLATE_NAME "TPTMS"
#define BLYNK_AUTH_TOKEN "NyDDlBRSYw3LTI1CHQI29emaaPoK56MF"
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "AMITH_2.4G";
char pass[] = "Amith@1212";

// BMP280 initialization
Adafruit_BMP280 bmp; // I2C

// Data structure to send
typedef struct {
  float temperature;
  float pressure; // In PSI
  float altitude;
} SensorData;

SensorData data;

// ESP-NOW Peer Address (Receiver ESP32's MAC Address)
uint8_t receiverAddress[] = {0xCC, 0x7B, 0x5C, 0x1E, 0xD1, 0x6C}; // CC:7B:5C:1E:D1:6C Replace with the receiver's MAC address

// Blynk Timer
BlynkTimer timer;

// Function to send sensor data via ESP-NOW
void sendViaESPNow() {
  // Send data to receiver ESP32
  esp_now_send(receiverAddress, (uint8_t *)&data, sizeof(data));
  Serial.println("Data sent via ESP-NOW");
}

// Function to send sensor data to Blynk
void sendToBlynk() {
  Blynk.virtualWrite(V0, data.temperature);
  Blynk.virtualWrite(V1, data.pressure); // Send pressure in PSI
  Blynk.virtualWrite(V2, data.altitude);

  Serial.println("Data sent to Blynk");
}

void setup() {
  Serial.begin(9600);

  // Initialize Wi-Fi
  WiFi.mode(WIFI_STA);
  Blynk.begin(auth, ssid, pass);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  // Add ESP-NOW peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize BMP280
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // Set up Blynk timer to send data every 2 seconds
  timer.setInterval(2000L, sendToBlynk);
}

void loop() {
  // Read sensor data
  data.temperature = bmp.readTemperature();
  data.pressure = bmp.readPressure() / 6894.76; // Convert pressure to PSI
  data.altitude = bmp.readAltitude(1013.25);

  // Send data via ESP-NOW
  sendViaESPNow();

  // Print data for debugging
  Serial.print("Temperature: ");
  Serial.print(data.temperature);
  Serial.println("°C");

  Serial.print("Pressure: ");
  Serial.print(data.pressure);
  Serial.println(" PSI");

  Serial.print("Altitude: ");
  Serial.print(data.altitude);
  Serial.println(" m");
  // Run Blynk and timer tasks
  Blynk.run();
  timer.run();
  delay(2000); // Delay between readings
}