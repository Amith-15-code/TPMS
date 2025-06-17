#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi credentials
char ssid[] = "AMITH_2.4G";
char password[] = "Amith@1212";

// Initialize LCD with I2C address, number of columns, and rows
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address if needed

// Structure to hold received sensor data
typedef struct {
    float temperature;
    float pressure;
    float altitude;
} SensorData;

SensorData receivedData;

// Callback function for ESP-NOW data reception
void onDataRecv(const esp_now_recv_info_t* info, const uint8_t* incomingData, int len) {
    if (len != sizeof(SensorData)) {
        Serial.println("Error: Received data length mismatch.");
        return;
    }

    // Copy the received data into the structure
    memcpy(&receivedData, incomingData, sizeof(receivedData));

    // Print received data and Wi-Fi details to Serial Monitor
    Serial.println("Received Data:");
    Serial.printf("Temperature: %.2f °C\n", receivedData.temperature);
    Serial.printf("Pressure: %.2f psi\n", receivedData.pressure);
    Serial.printf("Altitude: %.2f m\n", receivedData.altitude);

    // Print sender MAC address
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
             info->src_addr[0], info->src_addr[1], info->src_addr[2],
             info->src_addr[3], info->src_addr[4], info->src_addr[5]);
    Serial.printf("Sender MAC: %s\n", macStr);

    // Print signal strength (RSSI)
    Serial.printf("Signal Strength (RSSI): %d dBm\n", WiFi.RSSI());

    // Display received data on the LCD
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(receivedData.temperature, 1); // Display temperature (1 decimal)
    lcd.print(" C");

    lcd.setCursor(0, 1);
    lcd.print("Press: ");
    lcd.print(receivedData.pressure, 1); // Display pressure (1 decimal)
    lcd.print(" psi");
}

void setup() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Initialize LCD
    lcd.init();
    lcd.backlight();
    lcd.print("Connecting WiFi...");

    // Connect to Wi-Fi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Display Wi-Fi status on LCD
    lcd.clear();
    lcd.print("WiFi connected");

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed!");
        lcd.clear();
        lcd.print("ESP-NOW failed");
        while (true); // Stop if initialization fails
    }

    // Register the callback function for ESP-NOW
    esp_now_register_recv_cb(onDataRecv);
    Serial.println("Receiver ready");
}

void loop() {
    // Nothing to do here; ESP-NOW works asynchronously
}
