#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>

const int SCALE_DOUT_PIN = D2;
const int SCALE_SCK_PIN = D3;
const int calim = D1; 

HX711 scale(SCALE_DOUT_PIN, SCALE_SCK_PIN);

const char* ssid =         "virusin";
const char* password =    "clave666123abczzz";
const char* mqtt_server = "192.168.1.36";  

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msgCA[50];

char alim[50];
int value = 0;

void setup() {
  pinMode(calim, OUTPUT); 
  Serial.begin(115200);
  scale.set_scale();// factor de calibración
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

  if (strcmp(topic, "calim") == 0){
    
    if ((char)payload[0] == '0') {
       Serial.println("LOW");
      digitalWrite(calim, LOW);  

      snprintf (msgCA, 75, "0");
    } 
  
    if ((char)payload[0] == '1') {
      Serial.println("HIGH");
      digitalWrite(calim, HIGH); 

      snprintf (msgCA, 75, "1");
    }
  }
}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP8266Cliente4")) {
      Serial.println("connected");
      
      client.publish("calim", "0");
      client.publish("alim", "-1");
      
      client.subscribe("calim"); 
      client.subscribe("alim"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("intentando en 5 segundos");
    
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
    Serial.println(msgCA);
    client.publish("calim", msgCA);
    
    float weight = scale.get_units(1);
    
    char balim[75];
    String(weight,2).toCharArray(balim,75);
    
    snprintf(alim, 75,balim); 
    Serial.print("Alimento: ");
    Serial.print(alim);
    client.publish("alim", alim);
    Serial.println(); 

  } 
}
