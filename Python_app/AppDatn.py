import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.properties import ObjectProperty
from kivy.properties import StringProperty
from kivy.clock import Clock, mainthread
import paho.mqtt.client as mqtt

# MQTT protocol
topicSubscribe = ["status/datnta"]
topicPublish = "cmd/datnta" 
mqtt_server = "broker.hivemq.com"
mqtt_port = 1883
buttonState = [0,0,0,0]
# user = ""
# pw = ""
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    for topic in topicSubscribe:
        client.subscribe(topic)

# client.username_pw_set(user, pw)
# client = mqtt.Client()
# App GUI and logic
class MyGrid(Widget):
    button1 = ObjectProperty(None)
    button2 = ObjectProperty(None)
    button3 = ObjectProperty(None)
    button4 = ObjectProperty(None)

    @mainthread
    def update(self, button_id, state):
        if button_id == '1':
            if state == '1':
                self.button1.text = "Button 1 ON"
                self.button1.background_color = [240, 255, 0, 1]
                buttonState[0] = 1
            elif state == '0':
                self.button1.text = "Button 1 OFF"
                self.button1.background_color = [255, 255, 255, 1]
                buttonState[0] = 0
        if button_id == '2':
            if state == '1':
                self.button2.text = "Button 2 ON"
                buttonState[1] = 1
            elif state == '0':
                self.button2.text = "Button 2 OFF"
                buttonState[1] = 0
        if button_id == '3':
            if state == '1':
                self.button3.text = "Button 3 ON"
                buttonState[2] = 1
            elif state == '0':
                self.button3.text = "Button 3 OFF"
                buttonState[2] = 0
        if button_id == '4':
            if state == '1':
                self.button4.text = "Button 4 ON"
                buttonState[3] = 1
            elif state == '0':
                self.button4.text = "Button 4 OFF"
                buttonState[3] = 0
    def btn(self, bt_number):
        DATNApp().sendCmd(bt_number)
        

class DATNApp(App):
    client = mqtt.Client()
    def on_message(self, client, userdata, message):
        data = message.payload.decode("utf-8")
        self.root.update(data[0], data[2])
    def build(self):
        self.client.on_message = self.on_message
        self.client.connect(mqtt_server, mqtt_port, 60)
        self.client.loop_start()
        self.client.subscribe("status/datnta")
        return MyGrid()

    def sendCmd(self, bt_number):
        str_temp = "/" + str(bt_number) + "/"
        if buttonState[int(bt_number) - 1] == 0:
            str_temp += '1'
            self.client.publish(topicPublish, str_temp)
        else:
            str_temp += '0'
            self.client.publish(topicPublish, str_temp)
if __name__ == "__main__":
    DATNApp().run()