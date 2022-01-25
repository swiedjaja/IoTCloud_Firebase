#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "device.h"
#include "wifi_id.h"
#include "firebase_id.h"

FirebaseData fbdo;
FirebaseConfig fbConfig;
FirebaseData fbdoStream;

void WifiConnect();
void Firebase_Init(const String& streamPath);
void onFirebaseStream(FirebaseStream data);

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Booting...");
  WifiConnect();
  Serial.println("Connecting to Firebase...");
  Firebase_Init("cmd");
  Serial.println("System ready.");
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);
}

void loop() {
  if (millis() % 3000 ==0)
  {
    digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);
    Firebase.RTDB.setInt(&fbdo, "/data", millis());
    digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);
  }
}

void onFirebaseStream(FirebaseStream data)
{
  Serial.printf("onFirebaseStream: %s %s %s %s\n", data.streamPath().c_str(),
                data.dataPath().c_str(), data.dataType().c_str(),
                data.stringData().c_str());
}

void Firebase_Init(const String& streamPath)
{
  FirebaseAuth fbAuth;
  fbConfig.host = FIREBASE_HOST;
  fbConfig.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&fbConfig, &fbAuth);
  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  fbdo.setBSSLBufferSize(2*1024, 1024);
#endif

  fbdo.setResponseSize(1024);
  Firebase.RTDB.setwriteSizeLimit(&fbdo, "small");
  while (!Firebase.ready())
  {
    Serial.println("Connecting to firebase...");
    delay(1000);
  }
  String path = streamPath;
  if (Firebase.RTDB.beginStream(&fbdoStream, path.c_str()))
  {
    Serial.println("Firebase stream on "+ path);
    Firebase.RTDB.setStreamCallback(&fbdoStream, onFirebaseStream, 0);
  }
  else
    Serial.println("Firebase stream failed: "+fbdoStream.errorReason());
}

void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }  
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
