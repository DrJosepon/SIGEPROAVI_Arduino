#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT11.h>

const int ventilador =  D3; //2   
const int deshumedecedor =  D4; //2   
int pin=D2;
DHT11 dht11(pin);

const char* ssid =         "virusin";
const char* password =    "clave666123abczzz";
const char* mqtt_server = "192.168.1.36";   

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msgV[50];
char msgD[50];

char temp[50];
char hum[50];

int value = 0;

int encendido = 1;

void setup() {
  pinMode(ventilador, OUTPUT);     
  pinMode(deshumedecedor, OUTPUT); 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
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

  if (strcmp(topic, "vent") == 0){
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '0') {
       Serial.println("LOW");
      digitalWrite(ventilador, LOW);  
      snprintf (msgV, 75, "0");
    } 
  
    if ((char)payload[0] == '1') {
      Serial.println("HIGH");
      digitalWrite(ventilador, HIGH); 
  
      snprintf (msgV, 75, "1");
    }
  }

  if(strcmp(topic, "deshum") == 0 ){
    
    if ((char)payload[0] == '0') {
       Serial.println("LOW");
      digitalWrite(deshumedecedor, LOW);  
      snprintf (msgD, 75, "0");
    } 
  
    if ((char)payload[0] == '1') {
      Serial.println("HIGH");
      digitalWrite(deshumedecedor, HIGH);  
      snprintf (msgD, 75, "1");
    }
  }

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
  
    if (client.connect("ESP8266Cliente")) {
      Serial.println("connected");
     
      client.publish("vent", "0");
      client.publish("deshum", "0");
      client.publish("temp", "-1");
      client.publish("hum", "-1");
    
      client.subscribe("vent");
      client.subscribe("deshum");
      client.subscribe("temp");
      client.subscribe("hum");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("intentado en 5 segundos");
      
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
    Serial.println(msgV);
    Serial.println(msgD);
    client.publish("vent", msgV);
    client.publish("deshum", msgD);
    
     int err;
     float tempp, humm;
     if((err = dht11.read(humm, tempp)) == 0)    // Si devuelve 0 es que ha leido bien
        {
           char temi[75];
           String(tempp).toCharArray(temi,75);
           char humi[75];
           String(humm).toCharArray(humi,75);
           
           snprintf(temp, 7,temi);
           snprintf(hum, 75, humi);
           Serial.print("Temperatura: ");
           Serial.print(temp);
           client.publish("temp", temp);
           Serial.print(" Humedad: ");
           Serial.print(hum);
           client.publish("hum", hum);
           Serial.println();
        }
     else
        {
           Serial.println();
           Serial.print("Error Num :");
           Serial.print(err);
           Serial.println();
        }  
    
  }
}


