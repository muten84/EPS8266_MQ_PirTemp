#include <Wire.h>
#include <string.h>
#include <stdio.h>
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <PubSubClient.h>

#include <dht11.h>

#define pirPin D3
#define DHT11_PIN D4
#define READ_TEMP 100

const char* ssid = ".........";
const char* mqtt_server = ".....";

dht11 DHT;
WiFiClient espClient;
PubSubClient client(espClient);

int c = 0;
char msg[50];

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void readDHT() {
  Serial.print(DHT.humidity, 1);
  Serial.print(",\t");
  Serial.println(DHT.temperature, 1);
}

int queryPir() {
  int val = digitalRead(pirPin);
  return val;
}

int queryDHT() {
  int chk;
  chk = DHT.read(DHT11_PIN);    // READ DATA
  switch (chk) {
    case DHTLIB_OK:
      Serial.print("OK,\t");
      break;
    case DHTLIB_ERROR_CHECKSUM:
      Serial.print("Checksum error,\t");
      break;
    case DHTLIB_ERROR_TIMEOUT:
      Serial.print("Time out error,\t");
      break;
    default:
      Serial.print("Unknown error,\t");
      break;
  }
  return chk;
}

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(9600);
  Serial.println("Pir and Temperature Sensor");
  Serial.print("DHT LIBRARY VERSION: ");
  Serial.println(DHT11LIB_VERSION);
  Serial.println();
  Serial.println("Initializing PIR.....");
  delay(30000);
  Serial.println("...Initialized");
  Serial.println("Initializing Wi-Fi Network.....");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    Serial.println("Wi-fi reconnecting...");
    reconnect();
  }
 
  client.loop();


  int p = queryPir();
  if (p == HIGH) {
    Serial.println("Alert! Motion detected");
    snprintf (msg, 75, "Motion Detected #%ld", p);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
    delay(3000);
  }
  c++;
  if (c == READ_TEMP) {
    int s = queryDHT();
    c = 0;
    if (s == DHTLIB_OK) {
      readDHT();
    }
  }
  delay(100);
}

