/*
    Develop by      : Fahmi Nurfadilah
    Email           : fahmi.nurfadilah1412@gmail.com
    Updated by      : Vitradisa Pratama
    Email           : vitradisa@pptik.itb.ac.id
    Project         : IoT General
    Version         : 1.2
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>

// Update these with values suitable for your network.


const char* ssid = "TEKIDO";
const char* password = "iotworkshop2021";
const char* mqtt_server = "rmq2.pptik.id";
const char* mqtt_user = "/smarthome:smarthome";
const char* mqtt_pass = "Ssm4rt2!";
const char* CL = "LSKK-IWK-LAMPU-"; //ini diganti
int loop_count  = 0 ; //loop count loop

String statusDevice[8] = {"0"};

int relay1 = D1 ;

const char* device_guid ="GUID"; //ini juga
String output_value;

char msg[100];
WiFiClient espClient;
PubSubClient client(espClient);

byte mac[6];
String MACAddress;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  WiFi.mode(WIFI_STA);
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

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}

void printMACAddress() {
  WiFi.macAddress(mac);
  MACAddress = mac2String(mac);
  Serial.println(MACAddress);
}

char sPayload[100];
char message [40] ;
char address[40];
void callback(char* topic, byte* payload, unsigned int length) {
  memcpy(sPayload,payload,length);
  memcpy(address,payload,36);
  memcpy(message,&payload[37],length-37);
  Serial.print("Message arrived [");
  Serial.print(sPayload);
  Serial.println("] ");

  Serial.println(device_guid);
  Serial.println(address);
  if(String((char *)address) == String((char *)device_guid)) 
  {
    Serial.println("address sama");
  }
  else
  { 
    Serial.println("address berbeda");
    return;    
  }

  Serial.println(message);

  if (message[0] == '1') {
    digitalWrite(relay1, HIGH);
    Serial.println("relay 1 on");
    statusDevice[0] = "1";

  }
  if (message[0] == '0') {
    digitalWrite(relay1, LOW);
    Serial.println("relay 1 off");
    statusDevice[0] = "0";
  }


//  Serial.print("Publish message: ");
//  String pubmsg = String(device_guid) + "#" + String(statusDevice[0]);
//  Serial.println(pubmsg);
//  client.publish(mqtt_pub_topic, pubmsg.c_str());
//  delay(50);
}

void reconnect() {
  // Loop until we're reconnected
  printMACAddress();
  const char* CL;
  CL = MACAddress.c_str();
  Serial.println(CL);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(CL, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      client.subscribe("Aktuator");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(2000);

    }
  }
}


void watchdogSetup(void) {
  ESP.wdtDisable();
}


void setup()
{
  pinMode(D1, OUTPUT);

  digitalWrite(D1, LOW);

  //pinMode(input, INPUT);
  Serial.begin(115200);
  setup_wifi();
  printMACAddress();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  watchdogSetup();
}
void loop() {
  for (int i = 0; i <= loop_count; i++) {
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
  loop_count++;
  ESP.wdtFeed();
  Serial.print(loop_count);
  Serial.print(". Watchdog fed in approx. ");
  Serial.print(loop_count * 1000);
  Serial.println(" milliseconds.");
  delay(1000);
}
