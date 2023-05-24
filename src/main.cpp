// MAC: C8:F0:9E:9C:43:68 - sender
// MAC: E0:5A:1B:A1:9B:00 - receiver
// SENDER CODE - SENSOR SIDE§

#include <esp_now.h>
#include <WiFi.h>

#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xE0, 0x5A, 0x1B, 0xA1, 0x9B, 0x00};

// Default network credentials
char ssid[32] = "ESP32AP";
char password[64] = "0123456789";

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  char a[32];
  int b;
  float c;
  bool d;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void handleSaveCredentialsRequest(AsyncWebServerRequest *request)
{
  String submittedSSID = request->arg("ssid");
  String submittedPassword = request->arg("password");

  // Check if the submitted SSID is not empty and the password has at least 8 characters
  if (submittedSSID.length() > 0 && submittedPassword.length() >= 8 && submittedPassword.length() <= 64 && submittedSSID.length() <= 32 && submittedPassword != ssid && submittedSSID != password)
  {
    // Update the credentials
    strncpy(ssid, submittedSSID.c_str(), sizeof(ssid));
    strncpy(password, submittedPassword.c_str(), sizeof(password));

    request->send(SPIFFS, "/valid.html", "text/html");
    // Disconnect any connected clients
    WiFi.softAPdisconnect();
    // Configure ESP32 as an access point with new credentials
    WiFi.softAP(ssid, password);
    delay(1000);
  }
  else
  {
    request->send(SPIFFS, "/invalid-credentials.html", "text/html");
  }
}

void handleCredentialsRequest(AsyncWebServerRequest *request)
{
  String credentials = "SSID: " + String(ssid) + "\nPassword: " + String(password) + "\nMAC Address: " + WiFi.macAddress();
  request->send(200, "text/plain", credentials);
}

String convertMacToShortCode(const String &mac)
{
  const String characters = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz!@#$%^&*()";
  String shortCode = "";

  // Remove ':' characters from MAC address
  String cleanedMac = mac;
  cleanedMac.replace(":", "");

  // Convert MAC address to a short code
  for (int i = 0; i < 6; i++)
  {
    String pair = cleanedMac.substring(i * 2, i * 2 + 2);            // Extract each pair of MAC address numbers
    int decimalValue = strtoul(pair.c_str(), nullptr, 16);           // Convert the pair from hexadecimal to decimal
    char character = characters[decimalValue % characters.length()]; // Get the corresponding character from the character set
    shortCode += character;                                          // Append the character to the short code
  }

  return shortCode;
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station and Access Point
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid, password);

  // Init spiffs for web server
  if (!SPIFFS.begin())
  {
    Serial.println("Failed to mount SPIFFS");
    return;
  }

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully initialized, register for Send CB to get the status of transmitted packets
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }

  // Serve static files from SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server.on("/save-credentials", HTTP_POST, handleSaveCredentialsRequest);

  // Add a new endpoint for retrieving credentials
  server.on("/credentials", HTTP_GET, handleCredentialsRequest);

  // Start the server
  server.begin();

    // Get the MAC address
  String macAddress = WiFi.macAddress();

  // Convert MAC address to a short code
  String shortCode = convertMacToShortCode(macAddress);

  // Display the short code in the Serial Monitor
  Serial.print("Short Code: ");
  Serial.println(shortCode);
}

void loop()
{
  // Set values to send
  strcpy(myData.a, "THIS IS A CHAR");
  myData.b = random(1, 20);
  myData.c = 1.2;
  myData.d = false;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }
  delay(2000);
}
