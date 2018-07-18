#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>

const int SCALE_DOUT_PIN = D2;
const int SCALE_SCK_PIN = D3;
const int cbebi = D1; 

HX711 scale(SCALE_DOUT_PIN, SCALE_SCK_PIN);

const char* ssid =         "virusin";
const char* password =    "clave666123abczzz";
const char* mqtt_server = "192.168.1.36";   

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msgCB[50];

char bebi[50];
int value = 0;

void setup() {
  pinMode(cbebi, OUTPUT); 
  Serial.begin(115200);
  scale.set_scale();// <- factor de calibración
  scale.tare();

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}


void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

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

  if (strcmp(topic, "cbebi") == 0){
    
    if ((char)payload[0] == '0') {
       Serial.println("LOW");
      digitalWrite(cbebi, LOW);   
      
      snprintf (msgCB, 75, "0");
    } 
  
    if ((char)payload[0] == '1') {
      Serial.println("HIGH");
      digitalWrite(cbebi, HIGH);  
  
      snprintf (msgCB, 75, "1");
    }
  }
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP8266Cliente3")) {
      Serial.println("connected");
     
      client.publish("cbebi", "0");
      client.publish("bebi", "-1");
     
      client.subscribe("cbebi"); 
      client.subscribe("bebi"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" intentanto en 5 segundos");
    
      delay(5000);
    }
  }
}

void loop() {

 if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();

  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    
    Serial.print("Publish message: ");
    Serial.println(msgCB);
    client.publish("cbebi", msgCB);
    
    float weight = scale.get_units(1);
        
    char bbebi[75];
    String(weight,2).toCharArray(bbebi,75);
    
    snprintf(bebi, 75,bbebi); 
    Serial.print("Agua: ");
    Serial.print(bebi);
    client.publish("bebi", bebi);
    Serial.println(); 

  } 
}
