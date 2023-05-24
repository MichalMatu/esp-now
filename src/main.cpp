// MAC: C8:F0:9E:9C:43:68 - sender
// MAC: E0:5A:1B:A1:9B:00 - receiver
// SENDER CODE - SENSOR SIDE

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
  }
  else
  {
    request->send(SPIFFS, "/invalid-credentials.html", "text/html");
  }
}

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  // WiFi.mode(WIFI_STA);

  // Configure ESP32 as an access point
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

  

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
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

  // Start the server
  server.begin();
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