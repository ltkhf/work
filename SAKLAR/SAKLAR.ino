/*
    Develop by      : Fahmi Nurfadilah 
    Email           : fahmi.nurfadilah1412@gmail.com
    Project         : LSKK HomeAutomation
    Version         : 2.1
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

#if defined(ESP32)
    #error "Software Serial is not supported on the ESP32"
#endif

/* Use software serial for the PZEM
 * Pin 12 Rx (Connects to the Tx pin on the PZEM)
 * Pin 13 Tx (Connects to the Rx pin on the PZEM)
*/
#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN D2
#define PZEM_TX_PIN D1
#endif

SoftwareSerial pzemSWSerial(PZEM_RX_PIN, PZEM_TX_PIN);
PZEM004Tv30 pzem(pzemSWSerial);
// Update these with values suitable for your network.

const char* ssid = "TEKIDO";
const char* password = "iotworkshop2021";
const char* mqtt_server = "rmq2.pptik.id";
const char* mqtt_user = "/smarthome:smarthome";
const char* mqtt_pass = "Ssm4rt2!";
const char* CL = "LSKK-IWK-SAKLAR-"; //ini diganti
const char* mqtt_pub_topic = "Sensor";
const char* mqttQueuePublish = "Log";
int loop_count  = 0 ; //loop count loop

String statusDevice[8] = {"1"};

const int input1 = D7;
const int input2 = D6;

int prevState1 = 0;

char message [100];

WiFiClient espClient;
PubSubClient client(espClient);

byte mac[6];
String MACAddress;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
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
    } else {Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      ESP.restart();
      delay(5000);

    }
  }
}

void watchdogSetup(void) {
  ESP.wdtDisable();
}

void setup()
{
  Serial.begin(115200);
  pinMode(input1, INPUT_PULLUP);
  pinMode(input2, INPUT_PULLUP);
  setup_wifi();
  printMACAddress();
  client.setServer(mqtt_server, 1883);
  
  client.setCallback(callback);
  watchdogSetup();
}

String button_status[3];
String button_status2[3];

String prevpubmsg = "";
void loop() {
  String pubmsg = "";
  String input_guid = "GUID"; //ini diganti juga



  String dataSendtoMqtt;
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
        Serial.print("Custom Address:");
    Serial.println(pzem.readAddress(), HEX);

    // Read the data from the sensor
    float voltage = pzem.voltage();
    float current = pzem.current();
    float power = pzem.power();
    float energy = pzem.energy();
    float frequency = pzem.frequency();
    float pf = pzem.pf();

    // Check if the data is valid
    if(isnan(voltage)){
        Serial.println("Error reading voltage");
    } else if (isnan(current)) {
        Serial.println("Error reading current");
    } else if (isnan(power)) {
        Serial.println("Error reading power");
    } else if (isnan(energy)) {
        Serial.println("Error reading energy");
    } else if (isnan(frequency)) {
        Serial.println("Error reading frequency");
    } else if (isnan(pf)) {
        Serial.println("Error reading power factor");
    } else {

        // Print the values to the Serial console
        Serial.print("Voltage: ");      Serial.print(voltage);      Serial.println("V");
        Serial.print("Current: ");      Serial.print(current);      Serial.println("A");
        Serial.print("Power: ");        Serial.print(power);        Serial.println("W");
        Serial.print("Energy: ");       Serial.print(energy,3);     Serial.println("kWh");
        Serial.print("Frequency: ");    Serial.print(frequency, 1); Serial.println("Hz");
        Serial.print("PF: ");           Serial.println(pf);
    }

    Serial.println();
  delay(1000);
  
  int instate1 = digitalRead(input1);
  int instate2 = digitalRead(input2);
    
  if(instate1==LOW && instate2 ==LOW) button_status[1]="00";
  else if (instate1==HIGH && instate2 ==LOW) button_status[1]="10";
  else if (instate1==LOW && instate2 == HIGH) button_status[1]="01";
   else button_status[1]="11";

 
  dataSendtoMqtt = String(input_guid + "#" + voltage + "#" + current + "#" + power + "#" + energy + "#" + frequency + "#" + pf);
  client.publish(mqttQueuePublish, dataSendtoMqtt.c_str());
  Serial.println(dataSendtoMqtt);
  
  pubmsg = input_guid + "#" + button_status[1];
    //Kirim

  if (pubmsg != prevpubmsg) {
    client.publish(mqtt_pub_topic, pubmsg.c_str());
    Serial.println(pubmsg);
    prevpubmsg=pubmsg;
  }
    }
