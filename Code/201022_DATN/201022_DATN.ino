#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "nta3100";
const char* password = "20173616";
const char* mqtt_server = "broker.hivemq.com";
const int _10sec = 10* 1000;
const int _1min = 60 * 1000;
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
bool out1_state = LOW, out2_state = LOW, out3_state = LOW, out4_state = LOW;
bool long_pressed = false, check_long_press = false;
bool pressed = false;
int long_press_time = 0, bt_flag = 0;
long mqtt_rec, wifi_rec;

// ham doc nut an
void read_bt(void);
// ham thay doi trang thai relay
void change_output(void);
// ham set up wifi
void setup_wifi(int numper_try);
// ham nhan msg mqtt
void callback(char* topic, byte* payload, unsigned int length);
// ham reconnect server mqtt
void reconnect(int number_try);
// ham update trang thay hien tai cua switch
void update_state(void);
// ham xu li nut an giu
void long_press(void);
// ham isr
ICACHE_RAM_ATTR void isr_pressed(void){
  noInterrupts();
  delayMicroseconds(1000);
  if(digitalRead(bt1) == 1)
  {
    out1_state = !out1_state;
    bt_flag = 1;
  }
  if(digitalRead(bt2) == 1)
  {
    out2_state = !out2_state;
    bt_flag = 2;
  }
  if(digitalRead(bt3) == 1)
  {
    out3_state = !out3_state;
    bt_flag = 3;
  }
  if(digitalRead(bt4) == 1)
  {
    out4_state = !out4_state;
    bt_flag = 4;
  }
  change_output();
  digitalWrite(led_cf, HIGH);
  long_press_time = millis();
  check_long_press = true;
  while(check_long_press)
  {
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
      break;
    }
  }
  interrupts();
}

void setup() {
  // put your setup code here, to run once:
  // setup wifi
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

  setup_wifi(3);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
//  if(pressed == true)
//  {
//    read_bt();
//  }
//  if(check_long_press == true)
//  {
//    switch(bt_flag)
//    {
//      case 1:
//        if(digitalRead(bt1) == 0) check_long_press = false;
//        break;
//      case 2:
//        if(digitalRead(bt2) == 0) check_long_press = false;
//        break;
//      case 3:
//        if(digitalRead(bt3) == 0) check_long_press = false;
//        break;
//      case 4:
//        if(digitalRead(bt4) == 0) check_long_press = false;
//        break;
//      default:
//        break;
//    }
//    if(millis() - long_press_time > _10sec)
//    {
//      long_pressed = true;
//    }
//  }
//  if(long_pressed == true)
//  {
//    long_press();
//  }
  if(millis() - lastUpdate > _20sec)
  {
    lastUpdate = millis();
    if(client.connected())
    {
      update_state();
    }
  }
  
  if((WiFi.status() != WL_CONNECTED) && (millis() - wifi_rec < _1min))
  {
    wifi_rec = millis();
    setup_wifi(3);
    digitalWrite(led_cf, HIGH);
  }

  if((WiFi.status() == WL_CONNECTED) && (millis() - mqtt_rec < _1min) && !client.connected())
  {
    mqtt_rec = millis();
    reconnect(3);
    digitalWrite(led_cf, HIGH);
  }
  client.loop();
}

void read_bt(void)
{
  pressed = false;
  check_long_press = true;
  long_press_time = millis();
  delay(100);
  if(digitalRead(bt1) == 1)
  {
    out1_state = !out1_state;
    bt_flag = 1;
  }
  if(digitalRead(bt2) == 1)
  {
    out2_state = !out2_state;
    bt_flag = 2;
  }
  if(digitalRead(bt3) == 1)
  {
    out3_state = !out3_state;
    bt_flag = 3;
  }
  if(digitalRead(bt4) == 1)
  {
    out4_state = !out4_state;
    bt_flag = 4;
  }
  change_output();
}

void change_output(void)
{
  digitalWrite(out1, out1_state);
  digitalWrite(out2, out2_state);
  digitalWrite(out3, out3_state);
  digitalWrite(out4, out4_state);
  update_state();
}

void setup_wifi(int number_try)
{
  int try_times = 0;
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while ((WiFi.status() != WL_CONNECTED) && (try_times < number_try)) {
    digitalWrite(led_cf, LOW);
    delay(500);
    digitalWrite(led_cf, HIGH);
    delay(500);
    Serial.print(".");
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
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  if((char)payload[0] == '/')
  {
    if((char)payload[1] == '1')
    {
      if((char)payload[3] == '1') out1_state = HIGH;
      else out1_state = LOW;
    }
    else if ((char)payload[1] == '2')
    {
      if((char)payload[3] == '1') out2_state = HIGH;
      else out2_state = LOW;
    }
    else if ((char)payload[1] == '3')
    {
      if((char)payload[3] == '1') out3_state = HIGH;
      else out3_state = LOW;
    }
    else if ((char)payload[1] == '4')
    {
      if((char)payload[3] == '1') out4_state = HIGH;
      else out4_state = LOW;
    }
    change_output();
    update_state();
  }
}

void update_state(void)
{
  if(out1_state == 1)
  {
    client.publish("status/datnta","1/1");
  }
  else
  {
    client.publish("status/datnta","1/0");
  }
  
  if(out2_state == 1)
  {
    client.publish("status/datnta","2/1");
  }
  else
  {
    client.publish("status/datnta","2/0");
  }
  
  if(out3_state == 1)
  {
    client.publish("status/datnta","3/1");
  }
  else
  {
    client.publish("status/datnta","3/0");
  }
  
  if(out4_state == 1)
  {
    client.publish("status/datnta","4/1");
  }
  else
  {
    client.publish("status/datnta","4/0");
  }
}

void long_press(void)
{
  long_pressed = false;
  digitalWrite(led_cf, LOW);
  delayMicroseconds(_1min * 1000);
}
