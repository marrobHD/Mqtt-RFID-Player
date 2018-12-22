#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>

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

  while (!client.connected())
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
