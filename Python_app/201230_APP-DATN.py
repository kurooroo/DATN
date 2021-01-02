import kivy
from kivy.app import App
from kivy.uix.label import Label
from kivy.uix.gridlayout import GridLayout
from kivy.uix.button import Button
from kivy.uix.widget import Widget
from kivy.properties import ObjectProperty
from kivy.properties import StringProperty
from kivy.clock import Clock, mainthread
from kivy.base import runTouchApp
from kivy.lang import Builder
import webbrowser
import paho.mqtt.client as mqtt

debug = True
# MQTT protocol
topicSubscribe = ["status/datnta"]
topicPublish = "cmd/datnta" 
mqtt_server = "broker.hivemq.com"
mqtt_port = 1883
buttonState = [0,0,0,0]
start_up = True
flagConnected = False
# user = ""
# pw = ""
# def on_connect(client, userdata, flags, rc):
#     if debug:
#         print("Connected with result code "+str(rc))
#     global flagConnected
#     flagConnected = True
#     for topic in topicSubscribe:
#         client.subscribe(topic)

# def on_disconnect(client, userdata, rc):
#     global flagConnected
#     flagConnected = False

# client.username_pw_set(user, pw)
# client = mqtt.Client()
# App GUI and logic
class MyGrid(Widget):
    button1 = ObjectProperty(None)
    button2 = ObjectProperty(None)
    button3 = ObjectProperty(None)
    button4 = ObjectProperty(None)
    configWifi = ObjectProperty(None)
    @mainthread
    def update(self, data):
        buttonId = [data[1], data[5], data[9], data[13]]
        buttonUpdateState = [data[3], data[7], data[11], data[15]]
        if buttonUpdateState[0] == '1':
            self.button1.text = "Cong tac 1 Bat"
            buttonState[0] = 1
        elif buttonUpdateState[0] == '0':
            self.button1.text = "Cong tac 1 Tat"
            buttonState[0] = 0
        if buttonUpdateState[1] == '1':
            self.button2.text = "Cong tac 2 Bat"
            buttonState[1] = 1
        elif buttonUpdateState[1] == '0':
            self.button2.text = "Cong tac 2 Tat"
            buttonState[1] = 0
        if buttonUpdateState[2] == '1':
            self.button3.text = "Cong tac 3 Bat"
            buttonState[2] = 1
        elif buttonUpdateState[2] == '0':
            self.button3.text = "Cong tac 3 Tat"
            buttonState[2] = 0
        if buttonUpdateState[3] == '1':
            self.button4.text = "Cong tac 4 Bat"
            buttonState[3] = 1
        elif buttonUpdateState[3] == '0':
            self.button4.text = "Cong tac 4 Tat"
            buttonState[3] = 0

    def btn(self, bt_number):
        DATNApp().sendCmd(bt_number)
    def buttonConfigWifi(self):
        webbrowser.open('192.168.4.1/wifi?#p')
        # webbrowser.open('https://kivy.org/')
    
    Clock.schedule_interval(lambda dt: DATNApp().mqttReconnect(), 30)

class DATNApp(App):
    client = mqtt.Client()
    def on_connect(self, client, userdata, flags, rc):
        if debug:
            print("Connected with result code "+str(rc))
        global flagConnected
        flagConnected = True
        for topic in topicSubscribe:
            client.subscribe(topic)
    def on_disconnect(self, client, userdata, rc):
        global flagConnected
        flagConnected = False
        if debug:
            print(flagConnected)
    def on_message(self, client, userdata, message):
        data = message.payload.decode("utf-8")
        if debug:
            print(data)
        self.root.update(data)
        # self.root._update(data[0], data[2])
    def build(self):
        try:
            self.client.on_message = self.on_message
            self.client.on_connect = self.on_connect
            self.client.on_disconnect = self.on_disconnect
            self.client.connect(mqtt_server, mqtt_port, 60)
            self.client.loop_start()
            # self.client.subscribe("status/datnta")
            if debug:
                print("Connected")
        except:
            pass
        return MyGrid()

    def sendCmd(self, bt_number):
        str_temp = "/" + str(bt_number) + "/"
        if buttonState[int(bt_number) - 1] == 0:
            str_temp += '1'
        else:
            str_temp += '0'
        self.client.publish(topicPublish, str_temp)

    def mqttReconnect(self):
        if debug:
            print("clock")
        global flagConnected
        if flagConnected == False:
            if debug:
                print("try reconnect")
            try:
                # self.client.on_message = self.on_message
                self.client.connect(mqtt_server, mqtt_port, 60)
                self.client.loop_start()
                # self.client.subscribe("status/datnta")
            except:
                pass
if __name__ == "__main__":
    DATNApp().run()