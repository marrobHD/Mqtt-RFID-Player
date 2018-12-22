#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define SS_PIN D8
#define RST_PIN D1

MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long cardId = 0;

WiFiClient net;
PubSubClient client(net);

const char* mqtt_server = "technight.duckdns.org";
const char* ssid = "FRITZ!Box 6490 Cable_plus";
const char* password = "010166Hamburg";

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);

  client.setServer(mqtt_server, 1883);
}

void reconnect() {
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }

  while (!client.connected()) {
    String clientId = "NodeMCUClient-";
    clientId += String(random(0xffff), HEX);

    if (!client.connect(clientId.c_str(), "homeassistant-1", "t4KG95TT89pLSVSbkk")) {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void loop() {
  reconnect();

  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  cardId = getCardId();

  Serial.print("New ");
  Serial.println(cardId);

  char buffer[10];
  sprintf(buffer, "%lu", cardId);
  client.publish("/technight/Interface/PlayBox/Play", buffer);

  uint8_t control = 0x00;

  do {
    control = 0;
    for (int i = 0; i < 3; i++) {
      if (!mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }

        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }

        control += 0x1;
      }

      control += 0x4;
    }

    delay(0);
  } while (control == 13 || control == 14);

  reconnect();

  Serial.println("Removed");
  client.publish("/technight/Interface/PlayBox/Stop", "1");
  delay(500);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

unsigned long getCardId() {
  byte readCard[4];
  for (int i = 0; i < 4; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
  }

  return (unsigned long)readCard[0] << 24
    | (unsigned long)readCard[1] << 16
    | (unsigned long)readCard[2] << 8
    | (unsigned long)readCard[3];
}
