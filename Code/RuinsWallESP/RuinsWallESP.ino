
#include <WiFi.h>
#include <PubSubClient.h>

#define WIN_DETECTED_PIN 2
//************ GLOBAL VARIABLES **********
WiFiClient espClient;
PubSubClient mqtt(espClient);


const char * ssid = "AlchemyGuest";
const char * password = "VoodooVacation5601";
const char * mqttServer = "10.1.10.115";
const char * topic = "MermaidsTale/RuinsWallESP"; 



// *************** FUNCTIONS  *******************
//WIFI NETWORK
void setup_wifi() {
  delay(1000);
  Serial.println("*********** WIFI ***********");
  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);

  WiFi.begin(ssid,password);

  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-");
  }
  Serial.println("\nConnected.");
}

//MQTT SERVER
void reconnect() {
  while (!mqtt.connected()) {
    Serial.println("******** MQTT SERVER ********");
    if (mqtt.connect("ESP32 WROOM")) {
      Serial.print("Connection to broker established: ");
      Serial.println(mqttServer);

      mqtt.publish(topic, "Connected!");        //Message sent to broker, identifying the connected shell.
      mqtt.subscribe(topic);                             //shell subscribing to the broker's topic

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(". Trying again in 5 seconds.");
      delay(5000);
    }
  }
}

void setup_server(){
  mqtt.setServer(mqttServer, 1883);
}

//GENERAL FUNCTIONS
void setup_io(){
  pinMode(WIN_DETECTED_PIN,INPUT_PULLUP);
}
void winner(){
    mqtt.publish(topic,"RuinsWallSolved");
}
void program(){
  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  if(digitalRead(WIN_DETECTED_PIN))
    winner();
}

void _init(){
  //io setup
  setup_io();
  //network setup
  setup_wifi();
  //mqtt setup
  setup_server();
}

// ***************** SETUP *********************
void setup() {
  Serial.begin(115200);
  _init();
}

// **************** MAIN LOOP ****************
void loop() {
  program();
}



