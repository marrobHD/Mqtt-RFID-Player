
#include <EEPROM.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ESP8266WiFi.h>




// Connect to the WiFi
const char* ssid = "************";
const char* password = "**********";
// Connect to the MQTT Broker
const char* mqtt_server = "************";
const char* mqtt_username ="************";
const char* mqtt_password = "*************";
const char* clientID = "Alarm";


// this constant won't change:
const int buttonPin = D2;    // the pin that the pushbutton is attached to
const int chartsPin = D4;       // the pin that the LED is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


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
      digitalWrite(chartsPin, HIGH);
    if (receivedChar == '1')
      digitalWrite(chartsPin, LOW);
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID, mqtt_username, mqtt_password)) {
      Serial.println("Connected");
      // ... and subscribe to topic
      client.subscribe("ALARM/ZONE/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup()
{
  {

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    // initialize the button pin as a input:
    pinMode(buttonPin, INPUT);
    // initialize the LED as an output:
    pinMode(chartsPin, OUTPUT);
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

void loop() {

  // read the pushbutton input pin:
  buttonState = digitalRead(buttonPin);

  // compare the buttonState to its previous state
  if (buttonState != lastButtonState) {
    // if the state has changed, increment the counter
    if (buttonState == HIGH) {
      client.publish("ALARM/ZONE/testzone", "OFF"); //
      // if the current state is HIGH then the button
      // went from off to on:
    } else {
      // if the current state is LOW then the button
      // went from on to off:
      client.publish("ALARM/ZONE/testzone", "ON"); //
    }
    // Delay a little bit to avoid bouncing
    delay(100);
  }
  // save the current state as the last state,
  //for next time through the loop
  lastButtonState = buttonState;

  {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}
