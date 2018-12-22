#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

#define SS_PIN D8
#define RST_PIN D1

MFRC522 mfrc522(SS_PIN, RST_PIN);
unsigned long cardId = 0;


const char* ssid = "FRITZ!Box 6490 Cable_plus";
const char* password = "010166Hamburg";

const char* mqtt_server = "technight.duckdns.org";
const char* mqtt_username ="homeassistant-1";
const char* mqtt_password = "t4KG95TT89pLSVSbkk";
const char* clientID = "MusikBox";

// constants won't change. They're used here to set pin numbers:
const int buttonPin = 2;     // the number of the pushbutton pin
const int button2Pin = D3;     // the number of the pushbutton2 pin
//const int ledPin =  13;      // the number of the LED pin

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 1;     // previous state of the button

// Variables will change:
int button2PushCounter = 0;   // counter for the number of button presses
int button2State = 0;         // current state of the button
int lastButton2State = 1;     // previous state of the button

WiFiClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    Serial.print(receivedChar);
    if (receivedChar == '0')
      digitalWrite(LED_BUILTIN, HIGH);
    if (receivedChar == '1')
      digitalWrite(LED_BUILTIN, LOW);
  }
  Serial.println();
}


void setup()
{
  {
    SPI.begin();
    mfrc522.PCD_Init();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // initialize the button pin as a input:
    pinMode(buttonPin, INPUT);
    // initialize the LED as an output:
    pinMode(LED_BUILTIN, OUTPUT);
    // initialize serial communication:
    Serial.begin(9600);
  }

// Connect to WiFinetwork
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.begin(9600);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  //Serial.begin(9600);
  // Start the server
  // server.begin();
  // Serial.println("Server started");
  //Serial.begin(9600);
  // Print the IP address
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      // ... and subscribe to topic
      client.subscribe("/SmartHome/Interface/PlayBox/button");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

//  // read the state of the pushbutton value:
//  buttonState = digitalRead(buttonPin);
//
//  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
//  if (buttonState == HIGH) {
//    // turn LED on:
//    digitalWrite(LED_BUILTIN, HIGH);
//  } else {
//    // turn LED off:
//    digitalWrite(LED_BUILTIN, LOW);
//  }
//}
//

  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      client.publish("/SmartHome/Interface/PlayBox/button", "OFF"); //
      // if the current state is HIGH then the button
      // went from off to on:
    } else {
      // if the current state is LOW then the button
      // went from on to off:
      client.publish("/SmartHome/Interface/PlayBox/button", "ON"); //
    }
    // Delay a little bit to avoid bouncing
    delay(100);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;

  // read the pushbutton input pin:
  button2State = digitalRead(button2Pin);

  // compare the buttonState to its previous state
  if (button2State != lastButton2State) {
    // if the state has changed, increment the counter
    if (button2State == HIGH) {
      client.publish("/SmartHome/Interface/PlayBox/button2", "OFF"); //
      // if the current state is HIGH then the button
      // went from off to on:
    } else {
      // if the current state is LOW then the button
      // went from on to off:
      client.publish("/SmartHome/Interface/PlayBox/button2", "ON"); //
    }
    // Delay a little bit to avoid bouncing
    delay(100);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButton2State = button2State;


  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  cardId = getCardId();

  Serial.print("Message arrived [/SmartHome/Interface/PlayBox] ");
  Serial.println(cardId);

  char buffer[10];
  sprintf(buffer, "%lu", cardId);
  client.publish("/SmartHome/Interface/PlayBox", buffer);

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

  Serial.println("Message arrived [/SmartHome/Interface/PlayBox] 1");
  client.publish("/SmartHome/Interface/PlayBox", "1");
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
