#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <EEPROM.h>

#define DEBUG 1
char* ssid;
char* password;
const char* mqttServer = "broker.hivemq.com";
const int _10sec = 10*1000;
const int _1min = 60*1000;
const int _20sec = 20*1000;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastUpdate=0;

byte ledConfig = 16; //D0
byte bt1 = 5;  //D1
byte bt2 = 4;  //D2
byte bt3 = 14; //D5
byte bt4 = 12; //D6
byte out1 = 0;   //D3
byte out2 = 2;   //D4
byte out3 = 13;  //D7
byte out4 = 15;   //D8
bool outState[4] = {0, 0, 0, 0};
int switchTime[4] = {0, 0, 0, 0}, switchTimeRef[4] = {0, 0, 0, 0};
int buttonFlag;
bool longPressed = false, checkLongPress = false;
bool pressed = false;
int longPressTime = 0;
long mqttReconnect, wifiRec;

// ham doc nut an
// void read_bt(void);
// ham thay doi trang thai relay
void changeOutput(void);
// ham set up wifi
void setupWifi(int numberTry);
// ham nhan msg mqtt
void callback(char* topic,byte* payload, unsigned int length);
// ham reconnect server mqtt
void reconnect(int numberTry);
// ham update trang thay hien tai cua switch
// void update_state(void);
void updateState(void);
// ham xu li nut an giu
void longPress(void);
// ham blink led 0.5s/0.5s
void blinkLed(void);
// ham delay switch
void switchTimming(void);
// ham isr
ICACHE_RAM_ATTR void isrPressed(void){
  noInterrupts();
  delayMicroseconds(1000);
  if(digitalRead(bt1) == 1)
  {
    outState[0] = !outState[0];
    switchTime[0] = 0;
    buttonFlag = 1;
  }
  if(digitalRead(bt2) == 1)
  {
    outState[1] = !outState[1];
    switchTime[0] = 0;
    buttonFlag = 2;
  }
  if(digitalRead(bt3) == 1)
  {
    outState[2] = !outState[2];
    switchTime[2] = 0;
    buttonFlag = 3;
  }
  if(digitalRead(bt4) == 1)
  {
    outState[3] = !outState[3];
    switchTime[3] = 0;
    buttonFlag = 4;
  }
  changeOutput();
  longPressTime = millis();
  checkLongPress = true;
  // while(checkLongPress)
  // {
  //   switch(buttonFlag)
  //   {
  //     case 1:
  //       if(digitalRead(bt1) == 0) checkLongPress = false;
  //       break;
  //     case 2:
  //       if(digitalRead(bt2) == 0) checkLongPress = false;
  //       break;
  //     case 3:
  //       if(digitalRead(bt3) == 0) checkLongPress = false;
  //       break;
  //     case 4:
  //       if(digitalRead(bt4) == 0) checkLongPress = false;
  //       break;
  //     default:
  //       break;
  //   }
  //   if(millis() - longPressTime > _10sec)
  //   {
  //     #ifdef DEBUG
  //     Serial.print("longPress");
  //     #endif
  //     checkLongPress = false;
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
  pinMode(out2, OUTPUT);
  digitalWrite(out2, LOW);
  pinMode(out1, OUTPUT);
  digitalWrite(out1, LOW);
  pinMode(out3, OUTPUT);
  digitalWrite(out3, LOW);
  pinMode(out4, OUTPUT);
  digitalWrite(out4, LOW);
  Serial.begin(115200);
  pinMode(ledConfig, OUTPUT);
  digitalWrite(ledConfig, HIGH);
  pinMode(bt1, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt1), isrPressed, RISING);
  pinMode(bt2, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt2), isrPressed, RISING);
  pinMode(bt3, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt3), isrPressed, RISING);
  pinMode(bt4, INPUT);
  attachInterrupt(digitalPinToInterrupt(bt4), isrPressed, RISING);
//  pinMode(out1, OUTPUT);
//  digitalWrite(out1, LOW);
//  pinMode(out2, OUTPUT);
//  digitalWrite(out2, LOW);
//  pinMode(out3, OUTPUT);
//  digitalWrite(out3, LOW);
//  pinMode(out4, OUTPUT);
//  digitalWrite(out4, LOW);

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
    digitalWrite(ledConfig, LOW);
    WiFi.disconnect(true);
    WiFiManager wifiManager;
//    wifiManager.setAPStaticIPConfig();
//    wifiManager.autoConnect("DATN-NTA-20173616");
    wifiManager.setConfigPortalTimeout(180);
    wifiManager.setCaptivePortalEnable(false);
    wifiManager.setBreakAfterConfig(true);
    wifiManager.setConnectRetries(5);
    wifiManager.startConfigPortal("DATN-NTA-20173616");
    // ssid = wifiManager._ssid.c_str();
    // password = wifiManager._pass.c_str();
    ESP.restart();
  }
  else
  {
    setupWifi(10);
  }
  digitalWrite(ledConfig, HIGH);
  client.setServer(mqttServer, 1883);
  client.setCallback(callback);
  reconnect(2);
  // interrupts();
}

void loop() {
  // put your main code here, to run repeatedly:
//  if(pressed == true)
//  {
//    read_bt();
//  }
 if(checkLongPress == true)
 {
   #ifdef DEBUG 
   Serial.print("checkLongPress");
   #endif
   switch(buttonFlag)
   {
     case 1:
       if(digitalRead(bt1) == 0) checkLongPress = false;
       break;
     case 2:
       if(digitalRead(bt2) == 0) checkLongPress = false;
       break;
     case 3:
       if(digitalRead(bt3) == 0) checkLongPress = false;
       break;
     case 4:
       if(digitalRead(bt4) == 0) checkLongPress = false;
       break;
     default:
       break;
   }
   if(millis() - longPressTime > _10sec)
   {
     longPressed = true;
   }
 }
 if(longPressed == true)
 {
   longPress();
 }
  switchTimming();
  if(millis() - lastUpdate > _20sec)
  {
    lastUpdate = millis();
    if(client.connected())
    {
      updateState();
    }
  }
  
  if((WiFi.status() != WL_CONNECTED) && (millis() - wifiRec > _1min))
  {
    wifiRec = millis();
    setupWifi(3);
    digitalWrite(ledConfig, HIGH);
  }

  if((WiFi.status() == WL_CONNECTED) && (millis() - mqttReconnect > _1min) && !client.connected())
  {
    mqttReconnect = millis();
    reconnect(2);
    digitalWrite(ledConfig, HIGH);
  }
  client.loop();
}

// void read_bt(void)
// {
//   pressed = false;
//   checkLongPress = true;
//   longPressTime = millis();
//   delay(100);
//   if(digitalRead(bt1) == 1)
//   {
//     out1_state = !out1_state;
//     buttonFlag = 1;
//   }
//   if(digitalRead(bt2) == 1)
//   {
//     out2_state = !out2_state;
//     buttonFlag = 2;
//   }
//   if(digitalRead(bt3) == 1)
//   {
//     out3_state = !out3_state;
//     buttonFlag = 3;
//   }
//   if(digitalRead(bt4) == 1)
//   {
//     out4_state = !out4_state;
//     buttonFlag = 4;
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

void setupWifi(int numberTry)
{
  int tryTimes = 0;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WiFi.SSID());
  WiFi.begin();

  while ((WiFi.status() != WL_CONNECTED) && (tryTimes < numberTry)) {
    blinkLed();
    Serial.print(".");
    tryTimes++;
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

void reconnect(int numberTry)
{
  int tryTimes = 0;
  while((!client.connected()) && (tryTimes < numberTry))
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
      blinkLed();
      tryTimes++;
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  char timeBuffer[2];
  int timeTemp;
  timeBuffer[0] = (char)payload[5];
  timeBuffer[1] = (char)payload[6];
  timeTemp = atoi(timeBuffer);
  #ifdef DEBUG
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print(timeTemp);
  Serial.println();
  #endif
  if((char)payload[0] == '/')
  {
    if((char)payload[1] == '1')
    {
      switchTime[0] = timeTemp;
      switchTimeRef[0] = millis();
      if((char)payload[3] == '1')
      {
        outState[0] = 1;
      }
      else if(switchTime[0] == 0)
      {
        outState[0] = 0;
      }
    }
    else if ((char)payload[1] == '2')
    {
      switchTime[1] = timeTemp;
      switchTimeRef[1] = millis();
      if((char)payload[3] == '1')
      {
        outState[1] = 1;
      }
      else if(switchTime[1] == 0)
      {
        outState[1] = 0;
      }
    }
    else if ((char)payload[1] == '3')
    {
      switchTime[2] = timeTemp;
      switchTimeRef[2] = millis();
      if((char)payload[3] == '1')
      {
        outState[2] = 1;
      }
      else if(switchTime[2] == 0)
      {
        outState[2] = 0;
      }
    }
    else if ((char)payload[1] == '4')
    {
      switchTime[3] = timeTemp;
      switchTimeRef[3] = millis();
      if((char)payload[3] == '1')
      {
        outState[3] = 1;
      }
      else if(switchTime[3] == 0)
      {
        outState[3] = 0;
      }
    }
    changeOutput();
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

void longPress(void)
{
  longPressed = false;
  checkLongPress = false;
  #ifdef DEBUG
  Serial.print("longPressed");
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

void blinkLed(void)
{
  digitalWrite(ledConfig, LOW);
  delay(500);
  digitalWrite(ledConfig, HIGH);
  delay(500);
}

void switchTimming(void)
{
  for(int i = 0; i < 4; i++)
  {
    if(switchTime[i] != 0)
    {
      if(millis() - switchTimeRef[i] > switchTime[i]*1000)
      {
        switchTime[i] = 0;
        outState[i] = 0;
        changeOutput();
      }
    }
  }
}
