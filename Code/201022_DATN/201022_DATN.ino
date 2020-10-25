#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "nta3100";
const char* password = "20173616";
const char* mqtt_server = "";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg=0;

int button_pin1 = 5;  //D1
int button_pin2 = 4;  //D2
int button_pin3 = 14; //D5
int button_pin4 = 12; //D6
int relay_pin1 = 0;   //D3
int relay_pin2 = 2;   //D4
int relay_pin3 = 13;  //D7
int relay_pin4 = 1;   //TX
bool relay1_state = HIGH;
bool button1_pressed = false;

// ham doc nut an
void read_button(void);
// ham thay doi trang thai relay
void switch_relay(void);
// ham set up wifi
void setup_wifi(void);
// ham nhan msg mqtt
void callback(char* topic, byte* payload, unsigned int length);
// ham reconnect server mqtt
void reconnect(void);

void setup() {
  // put your setup code here, to run once:
  // setup wifi
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  
  pinMode(button_pin1, INPUT);
  pinMode(button_pin2, INPUT);
  pinMode(button_pin3, INPUT);
  pinMode(button_pin4, INPUT);
  pinMode(relay_pin1, OUTPUT);
  digitalWrite(relay_pin1, HIGH);
  pinMode(relay_pin2, OUTPUT);
  digitalWrite(relay_pin2, HIGH);
  pinMode(relay_pin3, OUTPUT);
  digitalWrite(relay_pin3, HIGH);
  pinMode(relay_pin4, OUTPUT);
  digitalWrite(relay_pin4, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected())
  {
    reconnect();
  }
  client.loop();
  read_button();
  switch_relay();
}

void read_button(void)
{
  if ((digitalRead(button_pin1) == 1) && (button1_pressed == false))
  {
    delay(100);
    if (digitalRead(button_pin1) == 1)
    {
      relay1_state = !relay1_state;
      button1_pressed = true;
    }
  }
  else
  {
    button1_pressed = false;
  }
}

void switch_relay(void)
{
  digitalWrite(relay_pin1, relay1_state);
}

void setup_wifi(void)
{
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

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect(void)
{
  while(!client.connected())
  {
    Serial.print("Attemp mqtt connection");
    String clientID = "esp8266-datn";
    if(client.connect(clientID.c_str()))
    {
      Serial.println("connected");
      client.subscribe();
    }
    else
    {
      Serial.print("fail, rc=");
      Serial.print(client.state());
      Serial.print("try again in 5sec");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  if((char)payload[0] == '1')
  {
    if((char)payload[2] == '1')
    {
      relay1_state = LOW;
    }
    else
    {
      relay1_state = HIGH;
    }
  }
}
