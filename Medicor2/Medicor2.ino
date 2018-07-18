#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// -- variables ------------------
const int analogInPin = A0;       
const int luz = D4; 
int retardo = 1 ;    
float lectura,ff,pKW,iA,vV,vS,S_Ratio;
 

const char* ssid =         "virusin";
const char* password =    "clave666123abczzz";
const char* mqtt_server = "192.168.1.36";  

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msgL[50];

char corr[50];
int value = 0;

// -- inicializar serial comm & parametros ------------
void setup() {
  pinMode(luz, OUTPUT); 
  Serial.begin(115200); 
  S_Ratio = 36.5;      // Sensor/ratio (mV/mA ) : 36.5
  vV = 220;            // valor de tension a computar
  ff = 4.15; // freq. factor / (50Hz -> 5 / 60Hz -> 4.15)  

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

  if (strcmp(topic, "luz") == 0){

    if ((char)payload[0] == '0') {
       Serial.println("LOW");
      digitalWrite(luz, LOW);  

      snprintf (msgL, 75, "0");
    } 
  
    if ((char)payload[0] == '1') {
      Serial.println("HIGH");
      digitalWrite(luz, HIGH);  

      snprintf (msgL, 75, "1");
    }
  }
}

void reconnect() {
 
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP8266Cliente2")) {
      Serial.println("connected");
      
      client.publish("luz", "0");
      client.publish("corr", "-1");
      
      client.subscribe("luz"); 
      client.subscribe("corr"); 
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      delay(5000);
    }
  }
}


// -- rutina de lecturacorriente ----------------------------
float lecturacorriente(){   
  int ni = 35;          // n. de iteraciones
  //  (ni) => rango 10 a 50 mejor promedio [smoothing]
  float retorno = 0.0;
  for (int x = 0; x< ni; x++){
  /*do {                         // espero paso por cero  
      delayMicroseconds(100); 
      } while (analogRead(0) != 0) ;*/
      delay (ff);            // espera centro de ciclo
      delay (10);            // estabilizacion CAD
      retorno = retorno + analogRead(0); 
    }
  return retorno / ni; 
}

// -- loop principal --------------------------------------
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
    Serial.println(msgL);
    client.publish("luz", msgL);
    
    lectura = lecturacorriente() / 1.41;    // lectura (rms)   
    vS = (lectura * 0.0048);          // valor de C.A.D.
    iA = (lectura * S_Ratio)/1000;     // Intensidad (A)
    pKW = (vV * iA)/1000;               // Potencia (kW)
    
    char corri[75];
    String(pKW,3).toCharArray(corri,75);
    
    snprintf(corr, 75,corri); 
    Serial.print("Corriente: ");
    Serial.print(corr);
    client.publish("corr", corr);
    Serial.println();

  }
                  
}
