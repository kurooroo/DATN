#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define DEBUG 1
char* ssid;
char* password;
const char* mqtt_server = "broker.hivemq.com";
const int _10sec = 10*1000;
const int _1min = 60*1000;
const int _20sec = 20*1000;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastUpdate=0;

byte led_cf = 16; //D0
byte bt1 = 5;  //D1
byte bt2 = 4;  //D2
byte bt3 = 14; //D5
byte bt4 = 12; //D6
byte out1 = 0;   //D3
byte out2 = 2;   //D4
byte out3 = 13;  //D7
byte out4 = 15;   //D8
bool outState[4] = {0, 0, 0, 0};
int bt_flag;
bool long_pressed = false, check_long_press = false;
bool pressed = false;
int long_press_time = 0;
long mqtt_rec, wifi_rec;

// ham doc nut an
// void read_bt(void);
// ham thay doi trang thai relay
void changeOutput(void);
// ham set up wifi
void setup_wifi(int numper_try);
// ham nhan msg mqtt
void callback(char* topic,byte* payload, unsigned int length);
// ham reconnect server mqtt
void reconnect(int number_try);
// ham update trang thay hien tai cua switch
// void update_state(void);
void updateState(void);
// ham xu li nut an giu
void long_press(void);
// ham isr
ICACHE_RAM_ATTR void isr_pressed(void){
  noInterrupts();
  delayMicroseconds(1000);
  if(digitalRead(bt1) == 1)
  {
    outState[0] = !outState[0];
    bt_flag = 1;
  }
  if(digitalRead(bt2) == 1)
  {
    outState[1] = !outState[1];
    bt_flag = 2;
  }
  if(digitalRead(bt3) == 1)
  {
    outState[2] = !outState[2];
    bt_flag = 3;
  }
  if(digitalRead(bt4) == 1)
  {
    outState[3] = !outState[3];
    bt_flag = 4;
  }
  changeOutput();
  long_press_time = millis();
  check_long_press = true;
  // while(check_long_press)
  // {
  //   switch(bt_flag)
  //   {
  //     case 1:
  //       if(digitalRead(bt1) == 0) check_long_press = false;
  //       break;
  //     case 2:
  //       if(digitalRead(bt2) == 0) check_long_press = false;
  //       break;
  //     case 3:
  //       if(digitalRead(bt3) == 0) check_long_press = false;
  //       break;
  //     case 4:
  //       if(digitalRead(bt4) == 0) check_long_press = false;
  //       break;
  //     default:
  //       break;
  //   }
  //   if(millis() - long_press_time > _10sec)
  //   {
  //     #ifdef DEBUG
  //     Serial.print("long_press");
  //     #endif
  //     check_long_press = false;
  //     EEPROM.begin(512);
  //     EEPROM.write(0, 'y');
  //     EEPROM.commit();
  //     EEPROM.end();
  //     ESP.restart();
  //     break;
  //   }
  // }
  interrupts();
}

void setup() {
  // put your setup code here, to run once:
  // setup wifi
  // noInterrupts();
  Serial.begin(115200);
  pinMode(led_cf, OUTPUT);
  digitalWrite(led_cf, HIGH);
  pinMode(bt1, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt1), isr_pressed, RISING);
  pinMode(bt2, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt2), isr_pressed, RISING);
  pinMode(bt3, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt3), isr_pressed, RISING);
  pinMode(bt4, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt4), isr_pressed, RISING);
  pinMode(out1, OUTPUT);
  digitalWrite(out1, LOW);
  pinMode(out2, OUTPUT);
  digitalWrite(out2, LOW);
  pinMode(out3, OUTPUT);
  digitalWrite(out3, LOW);
  pinMode(out4, OUTPUT);
  digitalWrite(out4, LOW);

  EEPROM.begin(512);
  char check = char(EEPROM.read(0));
  #ifdef DEBUG
  Serial.print("check = ");
  Serial.print(check);
  #endif
  EEPROM.write(0, 0);
  EEPROM.commit();
  EEPROM.end();
  if(check == 'y')
  {
    digitalWrite(led_cf, LOW);
    WiFi.disconnect(true);
    WiFiManager wifiManager;
//    wifiManager.setAPStaticIPConfig();
//    wifiManager.autoConnect("DATN-NTA-20173616");
    // wifiManager.setConfigPortalTimeout(180);
    wifiManager.startConfigPortal("DATN-NTA-20173616");
    // ssid = wifiManager._ssid.c_str();
    // password = wifiManager._pass.c_str();
  }
  else
  {
    setup_wifi(60);
  }
  digitalWrite(led_cf, HIGH);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // interrupts();
}

void loop() {
  // put your main code here, to run repeatedly:
//  if(pressed == true)
//  {
//    read_bt();
//  }
 if(check_long_press == true)
 {
   #ifdef DEBUG 
   Serial.print("check_long_press");
   #endif
   switch(bt_flag)
   {
     case 1:
       if(digitalRead(bt1) == 0) check_long_press = false;
       break;
     case 2:
       if(digitalRead(bt2) == 0) check_long_press = false;
       break;
     case 3:
       if(digitalRead(bt3) == 0) check_long_press = false;
       break;
     case 4:
       if(digitalRead(bt4) == 0) check_long_press = false;
       break;
     default:
       break;
   }
   if(millis() - long_press_time > _10sec)
   {
     long_pressed = true;
   }
 }
 if(long_pressed == true)
 {
   long_press();
 }
  if(millis() - lastUpdate > _20sec)
  {
    lastUpdate = millis();
    if(client.connected())
    {
      updateState();
    }
  }
  
  if((WiFi.status() != WL_CONNECTED) && (millis() - wifi_rec > _1min))
  {
    wifi_rec = millis();
    setup_wifi(3);
    digitalWrite(led_cf, HIGH);
  }

  if((WiFi.status() == WL_CONNECTED) && (millis() - mqtt_rec > _1min) && !client.connected())
  {
    mqtt_rec = millis();
    reconnect(3);
    digitalWrite(led_cf, HIGH);
  }
  client.loop();
}

// void read_bt(void)
// {
//   pressed = false;
//   check_long_press = true;
//   long_press_time = millis();
//   delay(100);
//   if(digitalRead(bt1) == 1)
//   {
//     out1_state = !out1_state;
//     bt_flag = 1;
//   }
//   if(digitalRead(bt2) == 1)
//   {
//     out2_state = !out2_state;
//     bt_flag = 2;
//   }
//   if(digitalRead(bt3) == 1)
//   {
//     out3_state = !out3_state;
//     bt_flag = 3;
//   }
//   if(digitalRead(bt4) == 1)
//   {
//     out4_state = !out4_state;
//     bt_flag = 4;
//   }
//   changeOutput();
// }

void changeOutput(void)
{
  digitalWrite(out1, outState[0]);
  digitalWrite(out2, outState[1]);
  digitalWrite(out3, outState[2]);
  digitalWrite(out4, outState[3]);
  updateState();
}

void setup_wifi(int number_try)
{
  int try_times = 0;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
  WiFi.begin();

  while ((WiFi.status() != WL_CONNECTED) && (try_times < number_try)) {
    digitalWrite(led_cf, LOW);
    delay(500);
    digitalWrite(led_cf, HIGH);
    delay(500);
    Serial.print(".");
    try_times++;
  }
  randomSeed(micros());
  if(WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("WiFi not connected");  
  }
}

void reconnect(int number_try)
{
  int try_times = 0;
  while((!client.connected()) && (try_times < number_try))
  {
    Serial.print("Attemp mqtt connection");
    String clientID = "esp8266-datn";
    if(client.connect(clientID.c_str()))
    {
      Serial.println("connected");
      client.publish("status/datnta", "connected");
//      client.subscribe("status/datnta");
      client.subscribe("cmd/datnta");
    }
    else
    {
      Serial.print("fail, rc=");
      Serial.print(client.state());
      Serial.print("try again in 5sec");
      try_times++;
      digitalWrite(led_cf, LOW);
      delay(1000);
      digitalWrite(led_cf, HIGH);
      delay(1000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  #ifdef DEBUG
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  #endif
  if((char)payload[0] == '/')
  {
    if((char)payload[1] == '1')
    {
      if((char)payload[3] == '1') outState[0] = 1;
      else outState[0] = 0;
    }
    else if ((char)payload[1] == '2')
    {
      if((char)payload[3] == '1') outState[1] = 1;
      else outState[1] = 0;
    }
    else if ((char)payload[1] == '3')
    {
      if((char)payload[3] == '1') outState[2] = 1;
      else outState[2] = 0;
    }
    else if ((char)payload[1] == '4')
    {
      if((char)payload[3] == '1') outState[3] = 1;
      else outState[3] = 0;
    }
    changeOutput();
    updateState();
  }
}

// void update_state(void)
// {
//   if(out1_state == 1)
//   {
//     client.publish("status/datnta","1/1");
//   }
//   else
//   {
//     client.publish("status/datnta","1/0");
//   }
  
//   if(out2_state == 1)
//   {
//     client.publish("status/datnta","2/1");
//   }
//   else
//   {
//     client.publish("status/datnta","2/0");
//   }
  
//   if(out3_state == 1)
//   {
//     client.publish("status/datnta","3/1");
//   }
//   else
//   {
//     client.publish("status/datnta","3/0");
//   }
  
//   if(out4_state == 1)
//   {
//     client.publish("status/datnta","4/1");
//   }
//   else
//   {
//     client.publish("status/datnta","4/0");
//   }
// }

void long_press(void)
{
  long_pressed = false;
  check_long_press = false;
  #ifdef DEBUG
  Serial.print("long_pressed");
  #endif
  EEPROM.begin(512);
  EEPROM.write(0, 'y');
  EEPROM.commit();
  EEPROM.end();
  ESP.restart();
}

void updateState(void)
{
  String mqttPubString;
  mqttPubString = "/1/" + String(outState[0]);
  mqttPubString += "/2/" + String(outState[1]);
  mqttPubString += "/3/" + String(outState[2]);
  mqttPubString += "/4/" + String(outState[3]);
  client.publish("status/datnta",(char*)mqttPubString.c_str());
}
