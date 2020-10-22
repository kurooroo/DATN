int button_pin1 = 5;
int button_pin2 = 4;
int button_pin3 = 14;
int button_pin4 = 12;
int relay_pin1 = 0;
int relay_pin2 = 2;
int relay_pin3 = 13;
int relay_pin4 = 1;
bool relay1_state = HIGH;
bool button1_pressed = false;

void setup() {
  // put your setup code here, to run once:
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
  // phan doc nut an
  if ((digitalRead(button_pin1) == 1) && (button1_pressed == false))
  {
    delay(10);
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
  // phan dieu khien relay
  digitalWrite(relay_pin1, relay1_state);
}
